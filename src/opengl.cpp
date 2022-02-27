#include "opengl.hpp"
#include "global.hpp"

#include <stdexcept>
#include <memory>

#include <GLFW/glfw3.h>

#include <direct.h>


Renderer::Renderer(){}

GLFWwindow* Renderer::initialize(int width, int height, int maxSamples)
{
	glfwInit();
		
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// 4.5版本OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_SAMPLES, 0);

	std::string tit = "PBR";
	GLFWwindow* window = glfwCreateWindow(width, height, tit.c_str(), nullptr, nullptr);
	if (!window) {
		throw std::runtime_error("GLFW窗口创建失败");
	}

	glfwMakeContextCurrent(window);
	// 设置监视器的最低刷新数
	glfwSwapInterval(-1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("");
	}
	// 最大各向异性层数
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &m_capabilities.maxAnisotropy);

#if _DEBUG
	glDebugMessageCallback(Renderer::logMessage, nullptr);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
	// 最大的MSAA子采样点数
	GLint maxSupportedSamples;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSupportedSamples);
	const int samples = glm::min(maxSamples, maxSupportedSamples);
	// 离屏MSAA渲染
	m_framebuffer = createFrameBufferWithRBO(width, height, samples, GL_RGBA16F, GL_DEPTH24_STENCIL8);
	if (samples > 0) {
		m_interFramebuffer = createFrameBufferWithRBO(width, height, 0, GL_RGBA16F, GL_NONE);
	}
	else {
		m_interFramebuffer = m_framebuffer;
	}
	m_shadowFrameBuffer = createShadowFrameBuffer(ShadowMapSize, ShadowMapSize);
	m_gbuffer = createGBuffer(ScreenWidth, ScreenHeight);
	

	std::printf("OpenGL 4.5 Renderer [%s]\n", glGetString(GL_RENDERER));

	// ImGui初始化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontDefault();
	ImFont* font = io.Fonts->AddFontFromFileTTF(
		"font/simhei.ttf", 17.0f, NULL,
		io.Fonts->GetGlyphRangesChineseFull()
	);
	//std::string iniPath = PROJECT_PATH + "/data/imgui.ini";
	//io.IniFilename = iniPath.c_str();
	//ImGui::LoadIniSettingsFromDisk(iniPath.c_str());

	IM_ASSERT(font != NULL);
	ImGui::GetIO().FontDefault = font;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	return window;
}

void Renderer::setup(const SceneSettings& scene)
{
	m_selectedIdx = -1;

	// 设置OpenGL全局状态
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glFrontFace(GL_CCW);

	// 创建屏幕VAO
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int quadVBO;
	glCreateVertexArrays(1, &m_quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(m_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 创建Uniform Buffer
	m_transformUB = createUniformBuffer<TransformUB>();
	m_shadingUB = createUniformBuffer<ShadingUB>();

	// 后处理、天空盒、pbr着色器
	std::string shaderPath = PROJECT_PATH + "/data/shaders";

	m_tonemapShader = Shader(shaderPath + "/postprocess_vs.glsl", shaderPath + "/postprocess_fs.glsl");
	m_pbrShader = Shader(shaderPath + "/pbr_vs.glsl", shaderPath + "/pbr_fs.glsl");
	m_skyboxShader = Shader(shaderPath + "/skybox_vs.glsl", shaderPath + "/skybox_fs.glsl");

	// prefilter、 irradianceMap、equirect计算着色器
	m_prefilterShader = ComputeShader(shaderPath + "/cs_prefilter.glsl");
	m_irradianceMapShader = ComputeShader(shaderPath + "/cs_irradiance_map.glsl");
	m_equirectToCubeShader = ComputeShader(shaderPath + "/cs_equirect2cube.glsl");

	// Shadow Map生成着色器
	m_dirLightShadowShader = Shader(
		shaderPath + "/shadow/directionalDepth_vs.glsl",
		shaderPath + "/shadow/directionalDepth_fs.glsl"
	);

	// 加载天空盒模型
	m_skybox = createMeshBuffer(Mesh::fromFiles(PROJECT_PATH + "/data/skybox.obj"));

	// 加载初始PBR模型以及贴图
	m_models.push_back(ModelPtr(Model::createPlane()));
	m_models[0]->rotation = Math::vec3(-90.0f, 0, 0);
	m_models[0]->scale = 6.0f;
	
	// 预计算高光部分需要的Look Up Texture (cosTheta, roughness)
	calcLUT();

	// 加载环境贴图，同时预计算prefilter以及irradiance map
	loadSceneHdr(scene.envName);
	
	// Shadow Map初始化
	initShadowMap(const_cast<SceneSettings&>(scene));

	glFinish();
}

void Renderer::render(GLFWwindow* window, const Camera& camera, const SceneSettings& scene)
{
	glViewport(0, 0, ScreenWidth, ScreenHeight);
	
	TransformUB transformUniforms;
	transformUniforms.view = camera.getViewMatrix();
	transformUniforms.projection =
		glm::perspective(
			glm::radians(camera.Zoom),
			float(m_framebuffer.width)/float(m_framebuffer.height),
			Near, Far
		);
	glNamedBufferSubData(m_transformUB, 0, sizeof(TransformUB), &transformUniforms);
	
	// TODO 封装成光照注册函数
	ShadingUB shadingUniforms;
	const glm::vec3 eyePosition = camera.Position;
	shadingUniforms.eyePosition = glm::vec4(eyePosition, 0.0f);
	for (int i = 0; i < SceneSettings::NumLights; ++i) {
		const DirectionalLight& light = scene.dirLights[i];
		shadingUniforms.lights[i].direction = glm::normalize(glm::vec4{ light.direction.toGlmVec(), 0.0f });
		if (light.enabled) {
			shadingUniforms.lights[i].radiance = glm::vec4{ light.radiance.toGlmVec(), 0.0f };
		}
		else {
			shadingUniforms.lights[i].radiance = glm::vec4{};
		}
		shadingUniforms.lights[i].lightSpaceMatrix = light.lightSpaceMatrix;

		const PointLight& ptLight = scene.ptLights[i];
		shadingUniforms.ptLights[i].position = glm::vec4(ptLight.position.toGlmVec(), 0.0f);
		if (ptLight.enabled) {
			shadingUniforms.ptLights[i].radiance = glm::vec4(ptLight.radiance.toGlmVec(), 0.0f);
		}
		else {
			shadingUniforms.ptLights[i].radiance = glm::vec4{};
		}
	}
	glNamedBufferSubData(m_shadingUB, 0, sizeof(ShadingUB), &shadingUniforms);
	
	// 渲染用的帧缓冲，接下来所有绘制的最后结果都会先保存在这个framebuffer上
	// 然后再将framebuffer绘制到屏幕上
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.id);
	
	glClearColor(scene.backgroundColor.x(), scene.backgroundColor.y(), scene.backgroundColor.z(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_transformUB);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_shadingUB);

	// TODO 封装天空盒绘制。后置天空盒，利用early Z
	// 天空盒
	/*if (scene.skybox) {
		m_skyboxShader.use();
		glDisable(GL_DEPTH_TEST);
		glBindTextureUnit(0, m_envTexture.id);
		glBindVertexArray(m_skybox.meshes[0]->vao);
		glDrawElements(GL_TRIANGLES, m_skybox.meshes[0]->numElements, GL_UNSIGNED_INT, 0);
	}*/

	
	// 模型
	m_pbrShader.use();
	m_pbrShader.setBool("haveSkybox", scene.skybox);
	// 如果不显示天空盒则为背景色
	m_pbrShader.setVec3("backgroundColor", scene.backgroundColor.toGlmVec());
	
	glBindTextureUnit(Model::TexCount, m_envTexture.id);
	glBindTextureUnit(Model::TexCount + 1, m_irmapTexture.id);
	glBindTextureUnit(Model::TexCount + 2, m_BRDF_LUT.id);
	glEnable(GL_DEPTH_TEST);
	
	for (int i = 0; i < scene.NumLights; ++i)
		glBindTextureUnit(Model::TexCount + 3 + i, scene.dirLights[i].shadowMap.id);

	for (int i = 0; i < m_models.size(); ++i) {
		if (!m_models[i]->visible) continue;
		transformUniforms.model =
			glm::translate(glm::mat4(1.0f), m_models[i]->position.toGlmVec()) *
			// TODO: 删去这一段测试用旋转代码
			glm::rotate(glm::mat4(1.0f), glm::radians(scene.objectPitch), glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(scene.objectYaw), glm::vec3(0, 1, 0)) *
			glm::eulerAngleXYZ(glm::radians(m_models[i]->rotation.x()), glm::radians(m_models[i]->rotation.y()), glm::radians(m_models[i]->rotation.z())) *
			glm::scale(glm::mat4(1.0f), glm::vec3(m_models[i]->scale));
		glNamedBufferSubData(m_transformUB, 0, sizeof(TransformUB), &transformUniforms);

		// TODO 把shader和纹理填装封装到模型的draw函数
		m_models[i]->draw(m_pbrShader);
	}

	// 天空盒
	if (scene.skybox) 
		drawSkybox();
	
	// 在后处理前将其复制到没有多重采样的中介framebuffer上
	resolveFramebuffer(m_framebuffer, m_interFramebuffer);

	// 将整个中介framebuffer绘制在屏幕上（在着色器中进行一些后处理）
	// 解绑先前的framebuffer，绑定回默认屏幕的framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_tonemapShader.use();
	glBindTextureUnit(0, m_interFramebuffer.colorTarget);
	glBindVertexArray(m_quadVAO);	// 屏幕VAO
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::drawSkybox()
{
	m_skyboxShader.use();
	glDepthFunc(GL_LEQUAL);
	glBindTextureUnit(0, m_envTexture.id);
	glBindVertexArray(m_skybox.meshes[0]->vao);
	glDrawElements(GL_TRIANGLES, m_skybox.meshes[0]->numElements, GL_UNSIGNED_INT, 0);
	glDepthFunc(GL_LESS);
}

void Renderer::shutdown()
{
	// TODO 检查一遍还有哪些变量要删除
	if (m_framebuffer.id != m_interFramebuffer.id) {
		deleteFrameBuffer(m_interFramebuffer);
	}
	deleteFrameBuffer(m_framebuffer);
	deleteFrameBuffer(m_shadowFrameBuffer);
	deleteFrameBuffer(m_gbuffer);

	glDeleteVertexArrays(1, &m_quadVAO);

	m_skyboxShader.deleteProgram();
	m_pbrShader.deleteProgram();
	m_tonemapShader.deleteProgram();
	m_prefilterShader.deleteProgram();
	m_irradianceMapShader.deleteProgram();
	m_dirLightShadowShader.deleteProgram();

	glDeleteBuffers(1, &m_transformUB);
	glDeleteBuffers(1, &m_shadingUB);

	deleteMeshBuffer(m_skybox);
	for (int i = 0; i < m_models.size(); ++i)
	{
		deleteModel(m_models[i]);
	}

	//deleteMeshBuffer(m_pbrModel);

	deleteTexture(m_envTexture);
	deleteTexture(m_irmapTexture);
	deleteTexture(m_BRDF_LUT);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Renderer::loadSceneHdr(const std::string& filename)
{
	// “尚未预滤波的环境贴图”变量（Cube Map类型)
	Texture envTextureUnfiltered = createTexture(GL_TEXTURE_CUBE_MAP, EnvMapSize, EnvMapSize, GL_RGBA16F);

	std::string envFilePath = PROJECT_PATH;
	envFilePath += "\\data\\hdr\\" + filename;
	envFilePath += ".hdr";

	Texture envTextureEquirect = createTexture(Image::fromFile(envFilePath, 3), GL_RGB, GL_RGB16F, 1);

	// equirectangular 投影，将得到的结果写入“尚未预滤波的环境贴图”中
	m_equirectToCubeShader.use();
	glBindTextureUnit(0, envTextureEquirect.id);
	glBindImageTexture(1, envTextureUnfiltered.id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	m_equirectToCubeShader.compute(
		envTextureUnfiltered.width / 32,
		envTextureUnfiltered.height / 32,
		6
	);

	// 已经投影到了envTextureUnfiltered上，将Equirect贴图删除
	deleteTexture(envTextureEquirect);

	// 为“尚未预滤波的环境贴图”自动生成mipmap链
	glGenerateTextureMipmap(envTextureUnfiltered.id);

	// 环境贴图（cube map型）
	m_envTexture = createTexture(GL_TEXTURE_CUBE_MAP, EnvMapSize, EnvMapSize, GL_RGBA16F);
	// 复制“尚未预滤波的环境贴图”mipmap第0层（原图）到环境贴图的第0层上
	glCopyImageSubData(envTextureUnfiltered.id, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
		m_envTexture.id, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
		m_envTexture.width, m_envTexture.height, 6);

	// 滤波环境贴图变量
	m_prefilterShader.use();
	glBindTextureUnit(0, envTextureUnfiltered.id);
	// 根据粗糙度不同，对环境贴图进行预滤波（从第1级mipmap开始，第0级是原图）
	const float maxMipmapLevels = glm::max(float(m_envTexture.levels - 1), 1.0f);
	int size = EnvMapSize / 2;
	for (int level = 1; level <= maxMipmapLevels; ++level) {
		const GLuint numGroups = glm::max(1, size / 32);
		// 将指定层级的纹理贴图绑定
		glBindImageTexture(1, m_envTexture.id, level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		m_prefilterShader.setFloat("roughness", (float)level / maxMipmapLevels);
		m_prefilterShader.compute(numGroups, numGroups, 6);
		size /= 2;
	}

	// 滤波过后的环境贴图已经存在m_envTexture里
	// 删除掉原有的“尚未预滤波的环境贴图”
	deleteTexture(envTextureUnfiltered);

	// 预计算漫反射用的 irradiance map.

	m_irmapTexture = createTexture(GL_TEXTURE_CUBE_MAP, IrradianceMapSize, IrradianceMapSize, GL_RGBA16F, 1);

	m_irradianceMapShader.use();
	glBindTextureUnit(0, m_envTexture.id);
	glBindImageTexture(1, m_irmapTexture.id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	m_irradianceMapShader.compute(
		m_irmapTexture.width / 32,
		m_irmapTexture.height / 32,
		6
	);
}

void Renderer::calcLUT() 
{
	// 预计算高光部分需要的Look Up Texture (cosTheta, roughness)
	ComputeShader LUTShader = ComputeShader("data/shaders/cs_lut.glsl");

	m_BRDF_LUT = createTexture(GL_TEXTURE_2D, BRDF_LUT_Size, BRDF_LUT_Size, GL_RG16F, 1);
	glTextureParameteri(m_BRDF_LUT.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_BRDF_LUT.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	LUTShader.use();
	glBindImageTexture(0, m_BRDF_LUT.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	LUTShader.compute(
		m_BRDF_LUT.width / 32,
		m_BRDF_LUT.height / 32,
		1
	);
	LUTShader.deleteProgram();
}


#if _DEBUG
void Renderer::logMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
		std::fprintf(stderr, "GL: %s\n", message);
	}
}
#endif


