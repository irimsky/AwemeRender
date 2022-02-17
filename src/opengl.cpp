#include "opengl.hpp"

#include <stdexcept>
#include <memory>

#include <GLFW/glfw3.h>
#include "global.hpp"
#include <direct.h>


struct TransformUB
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

struct ShadingUB
{
	struct {
		glm::vec4 direction;
		glm::vec4 radiance;
	} lights[SceneSettings::NumLights];
	struct {
		glm::vec4 position;
		glm::vec4 radiance;
	} ptLights[SceneSettings::NumLights];
	glm::vec4 eyePosition;
};

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

	GLFWwindow* window = glfwCreateWindow(width, height, "PBR", nullptr, nullptr);
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

	m_framebuffer = createFrameBuffer(width, height, samples, GL_RGBA16F, GL_DEPTH24_STENCIL8);
	if (samples > 0) {
		m_resolveFramebuffer = createFrameBuffer(width, height, 0, GL_RGBA16F, GL_NONE);
	}
	else {
		m_resolveFramebuffer = m_framebuffer;
	}

	std::printf("OpenGL 4.5 Renderer [%s]\n", glGetString(GL_RENDERER));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontDefault();
	ImFont* font = io.Fonts->AddFontFromFileTTF(
		"font/simhei.ttf", 17.0f, NULL,
		io.Fonts->GetGlyphRangesChineseFull()
	);
	
	IM_ASSERT(font != NULL);
	ImGui::GetIO().FontDefault = font;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	return window;
}

void Renderer::shutdown()
{
	if (m_framebuffer.id != m_resolveFramebuffer.id) {
		deleteFrameBuffer(m_resolveFramebuffer);
	}
	deleteFrameBuffer(m_framebuffer);

	glDeleteVertexArrays(1, &m_quadVAO);

	m_skyboxShader.deleteProgram();
	m_pbrShader.deleteProgram();
	m_tonemapShader.deleteProgram();
	m_prefilterShader.deleteProgram();
	m_irradianceMapShader.deleteProgram();

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

	/*deleteTexture(m_albedoTexture);
	deleteTexture(m_normalTexture);
	deleteTexture(m_metalnessTexture);
	deleteTexture(m_roughnessTexture);
	deleteTexture(m_emissionTexture);
	deleteTexture(m_occlusionTexture);*/

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Renderer::setup(const SceneSettings& scene)
{
	// 各种贴图大小
	m_EnvMapSize = 1024;	// 必须是2的次幂
	m_IrradianceMapSize = 32;
	m_BRDF_LUT_Size = 512;

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

	// 加载后处理、天空盒、pbr着色器
	
	m_tonemapShader = Shader("./data/shaders/postprocess_vs.glsl", "./data/shaders/postprocess_fs.glsl");
	m_pbrShader = Shader("./data/shaders/pbr_vs.glsl", "./data/shaders/pbr_fs.glsl");
	m_skyboxShader = Shader("./data/shaders/skybox_vs.glsl", "./data/shaders/skybox_fs.glsl");

	// 加载prefilter、 irradianceMap、equirect Project计算着色器
	m_prefilterShader = ComputeShader("./data/shaders/cs_prefilter.glsl");
	m_irradianceMapShader = ComputeShader("./data/shaders/cs_irradiance_map.glsl");
	m_equirectToCubeShader = ComputeShader("./data/shaders/cs_equirect2cube.glsl");

	std::cout << "Start Loading Models:" << std::endl;
	// 加载天空盒模型
	m_skybox = createMeshBuffer(Mesh::fromFile("./data/skybox.obj"));

	// 加载PBR模型以及贴图
	//loadModels(scene.objName, const_cast<SceneSettings&>(scene));

	m_models.push_back(Model("E:\\Code\\OpenGL\\AwemeRender\\data\\models\\helmet\\helmet.obj", true));
	m_models.push_back(Model("E:\\Code\\OpenGL\\AwemeRender\\data\\models\\cerberus\\cerberus.obj", true));
	m_models[1].position = Math::vec3(1.0f);
	
	
	// 预计算高光部分需要的Look Up Texture (cosTheta, roughness)
	calcLUT();

	// 加载环境贴图，同时预计算prefilter以及irradiance map
	loadSceneHdr(scene.envName);
		
	glFinish();
}

void Renderer::render(GLFWwindow* window, const Camera& camera, const SceneSettings& scene)
{
	
	TransformUB transformUniforms;
	transformUniforms.view = camera.GetViewMatrix();
	transformUniforms.projection = glm::perspective(glm::radians(camera.Zoom), float(m_framebuffer.width)/float(m_framebuffer.height), 0.1f, 1000.0f);
	glNamedBufferSubData(m_transformUB, 0, sizeof(TransformUB), &transformUniforms);
	
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
	glClear(GL_DEPTH_BUFFER_BIT); 

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_transformUB);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_shadingUB);

	// 天空盒
	m_skyboxShader.use();
	glDisable(GL_DEPTH_TEST);
	glBindTextureUnit(0, m_envTexture.id);
	glBindVertexArray(m_skybox.vao);
	glDrawElements(GL_TRIANGLES, m_skybox.numElements, GL_UNSIGNED_INT, 0);

	// 模型
	m_pbrShader.use();
	glEnable(GL_DEPTH_TEST);

	glBindTextureUnit(Model::TexCount, m_envTexture.id);
	glBindTextureUnit(Model::TexCount+1, m_irmapTexture.id);
	glBindTextureUnit(Model::TexCount+2, m_BRDF_LUT.id);

	for (int i = 0; i < m_models.size(); ++i) {
		transformUniforms.model = glm::translate(glm::mat4(1.0f), m_models[i].position.toGlmVec()) *
			glm::eulerAngleXY(glm::radians(scene.objectPitch), glm::radians(scene.objectYaw))
			* glm::scale(glm::mat4(1.0f), glm::vec3(m_models[i].scale));
		glNamedBufferSubData(m_transformUB, 0, sizeof(TransformUB), &transformUniforms);
		
		for (int j = 0; j < Model::TexCount; ++j)
		{
			std::string typeName = TextureTypeNames[j];
			if (m_models[i].haveTexture((TextureType)j))
			{
				m_pbrShader.setBool("have"+typeName, true);
				glBindTextureUnit(j, m_models[i].textures[j].id);
			}
			else
			{
				m_pbrShader.setBool("have" + typeName, false);
				if (j == (int)TextureType::Albedo)
					m_pbrShader.setVec3("commonColor", m_models[i].color.toGlmVec());
			}
		}
		/*if (m_models[i].haveAlbedo())
		{
			m_pbrShader.setBool("haveAlbedo", true);
			glBindTextureUnit(0, m_models[i].albedoTexture.id);
		}
		else
		{
			m_pbrShader.setBool("haveAlbedo", false);
			m_pbrShader.setVec3("commonColor", m_models[i].color.toGlmVec());
		}

		if (m_models[i].haveNormal())
		{
			m_pbrShader.setBool("haveNormal", true);
			glBindTextureUnit(1, m_models[i].normalTexture.id);
		}
		else
		{
			m_pbrShader.setBool("haveNormal", false);
		}

		if (m_models[i].haveMetalness())
		{
			m_pbrShader.setBool("haveMetalness", true);
			glBindTextureUnit(2, m_models[i].metalnessTexture.id);
		}
		else
		{
			m_pbrShader.setBool("haveMetalness", false);
		}

		if (m_models[i].haveRoughness())
		{
			m_pbrShader.setBool("haveRoughness", true);
			glBindTextureUnit(3, m_models[i].roughnessTexture.id);
		}
		else
		{
			m_pbrShader.setBool("haveRoughness", false);
		}

		if (m_models[i].haveOcclusion())
		{
			m_pbrShader.setBool("haveOcclusion", true);
			glBindTextureUnit(7, m_models[i].occlusionTexture.id);
		}
		else
		{
			m_pbrShader.setBool("haveOcclusion", false);
		}

		if (m_models[i].haveEmmission())
		{
			m_pbrShader.setBool("haveEmission", true);
			glBindTextureUnit(8, m_models[i].emissionTexture.id);
		}
		else
		{
			m_pbrShader.setBool("haveEmission", false);
		}

		if (m_models[i].haveHeight())
		{
			m_pbrShader.setBool("haveHeight", true);
			glBindTextureUnit(9, m_models[i].heightTexture.id);
		}
		else
		{
			m_pbrShader.setBool("haveHeight", false);
		}*/

		glBindVertexArray(m_models[i].pbrModel.vao);
		glDrawElements(GL_TRIANGLES, m_models[i].pbrModel.numElements, GL_UNSIGNED_INT, 0);
		
	}
	
	// 多重采样
	resolveFramebuffer(m_framebuffer, m_resolveFramebuffer);

	// 将整个framebuffer绘制在屏幕上（中间在着色器中进行一些后处理）
	// 解绑先前的framebuffer，绑定回默认屏幕的framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_tonemapShader.use();
	glBindTextureUnit(0, m_resolveFramebuffer.colorTarget);
	glBindVertexArray(m_quadVAO);	// 屏幕VAO
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// glfwSwapBuffers(window);
}


GLuint Renderer::createUniformBuffer(const void* data, size_t size)
{
	GLuint ubo;
	glCreateBuffers(1, &ubo);
	glNamedBufferStorage(ubo, size, data, GL_DYNAMIC_STORAGE_BIT);
	return ubo;
}

void Renderer::loadModels(const std::string& modelName, SceneSettings& scene)
{
	deleteMeshBuffer(m_pbrModel);
	deleteTexture(m_albedoTexture);
	deleteTexture(m_normalTexture);
	deleteTexture(m_metalnessTexture);
	deleteTexture(m_roughnessTexture);
	deleteTexture(m_emissionTexture);
	deleteTexture(m_occlusionTexture);
	deleteTexture(m_heightTexture);

	std::string modelPath = PROJECT_PATH;
	modelPath += "data/models/";
	modelPath += modelName;

	std::vector<char*> modelFiles = File::readAllFilesInDirWithExt(modelPath);
	bool haveMesh = false, haveTexture = false;
	for (char* str : modelFiles)
	{
		std::string tmpStr = str;
		int dotIdx = tmpStr.find_last_of('.');
		std::string name = tmpStr.substr(0, dotIdx), 
				    extName = tmpStr.substr(dotIdx);
		
		if (name == modelName)
		{
			scene.objExt = extName;
			haveMesh = true;
		}
		else if (name.substr(0, tmpStr.find_last_of('_')) == modelName)
		{
			scene.texExt = extName;
			haveTexture = true;
		}
	}

	if (modelFiles.size() == 0 || (!haveMesh && !haveTexture))
	{
		throw std::runtime_error("Failed to load model files: " + modelName);
	}

	std::string name = modelName;
	if (name.substr(name.find_last_of('_') + 1) == "ball")
		scene.objType = Mesh::Ball;
	else
		scene.objType = Mesh::ImportModel;

	modelPath += "/" + modelName;
	if (scene.objType == Mesh::ImportModel)
		m_pbrModel = createMeshBuffer(Mesh::fromFile(modelPath + scene.objExt));

	if (modelName == "cerberus")
		scene.objectScale = 2.2f;
	else
		scene.objectScale = 1.0f;

	// 加载纹理贴图
	std::cout << "Start Loading Textures:" << std::endl;
	m_albedoTexture = createTexture(
		Image::fromFile(modelPath + "_albedo" + scene.texExt, 3),
		GL_RGB, GL_SRGB8
	);
	m_normalTexture = createTexture(
		Image::fromFile(modelPath + "_normal" + scene.texExt, 3),
		GL_RGB, GL_RGB8
	);

	m_pbrShader.use();
	try {
		m_metalnessTexture = createTexture(
			Image::fromFile(modelPath + "_metalness" + scene.texExt, 1),
			GL_RED, GL_R8
		);
		m_pbrShader.setBool("haveMetalness", true);
	}
	catch (std::runtime_error) {
		std::cout << "No Metal Texture" << std::endl;
		m_pbrShader.setBool("haveMetalness", false);
	}

	try {
		m_roughnessTexture = createTexture(
			Image::fromFile(modelPath + "_roughness" + scene.texExt, 1),
			GL_RED, GL_R8
		);
		m_pbrShader.setBool("haveRoughness", true);
	}
	catch (std::runtime_error) {
		std::cout << "No Rough Texture" << std::endl;
		m_pbrShader.setBool("haveRoughness", false);
	}

	try {
		m_occlusionTexture = createTexture(
			Image::fromFile(modelPath + "_occlusion" + scene.texExt, 1),
			GL_RED, GL_R8
		);
		m_pbrShader.setBool("haveOcclusion", true);
	}
	catch (std::runtime_error) {
		std::cout << "No Occlusion Texture" << std::endl;
		m_pbrShader.setBool("haveOcclusion", false);
	}

	try {
		m_emissionTexture = createTexture(
			Image::fromFile(modelPath + "_emission" + scene.texExt, 3),
			GL_RGB, GL_SRGB8
		);
		m_pbrShader.setBool("haveEmission", true);
	}
	catch (std::runtime_error) {
		std::cout << "No Emission Texture" << std::endl;
		m_pbrShader.setBool("haveEmission", false);
	} 

	try {
		m_heightTexture = createTexture(
			Image::fromFile(modelPath + "_height" + scene.texExt, 1),
			GL_RED, GL_R8
		);
		m_pbrShader.setBool("haveHeight", true);
	}
	catch (std::runtime_error) {
		std::cout << "No Height Texture" << std::endl;
		m_pbrShader.setBool("haveHeight", false);
	}
 }

void Renderer::loadSceneHdr(const std::string& filename)
{
	// “尚未预滤波的环境贴图”变量（Cube Map类型)
	Texture envTextureUnfiltered = createTexture(GL_TEXTURE_CUBE_MAP, m_EnvMapSize, m_EnvMapSize, GL_RGBA16F);

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
	m_envTexture = createTexture(GL_TEXTURE_CUBE_MAP, m_EnvMapSize, m_EnvMapSize, GL_RGBA16F);
	// 复制“尚未预滤波的环境贴图”mipmap第0层（原图）到环境贴图的第0层上
	glCopyImageSubData(envTextureUnfiltered.id, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
		m_envTexture.id, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
		m_envTexture.width, m_envTexture.height, 6);

	// 滤波环境贴图变量
	m_prefilterShader.use();
	glBindTextureUnit(0, envTextureUnfiltered.id);
	// 根据粗糙度不同，对环境贴图进行预滤波（从第1级mipmap开始，第0级是原图）
	const float maxMipmapLevels = glm::max(float(m_envTexture.levels - 1), 1.0f);
	int size = m_EnvMapSize / 2;
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

	m_irmapTexture = createTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMapSize, m_IrradianceMapSize, GL_RGBA16F, 1);

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

	m_BRDF_LUT = createTexture(GL_TEXTURE_2D, m_BRDF_LUT_Size, m_BRDF_LUT_Size, GL_RG16F, 1);
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


