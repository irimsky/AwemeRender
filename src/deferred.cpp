#include "opengl.hpp"

void Renderer::deferredRender(GLFWwindow* window, const Camera& camera, const SceneSettings& scene)
{
	glViewport(0, 0, ScreenWidth, ScreenHeight);
	// TODO 封装两个Pass
	// 1. 几何计算Pass

	// TAA UB
	static int frameCount = -1;
	++frameCount;
	int haltonIdx = (frameCount + 1) % 8;
	
	taaUniforms.offsetIdx = haltonIdx;
	taaUniforms.screenHeight = ScreenHeight;
	taaUniforms.screenWidth = ScreenWidth;
	if (taaUniforms.preProjection != preProj)
	{
		int x = 1;
	}
	taaUniforms.preProjection = preProj;

	if (taaUniforms.preView != preView)
	{
		int x = 1;
	}
	taaUniforms.preView = preView;
	glNamedBufferSubData(m_taaUB, 0, sizeof(TaaUB), &taaUniforms);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_taaUB);

	// Transform UB
	transformUniforms.view = camera.getViewMatrix();
	transformUniforms.projection =
		glm::perspective(
			glm::radians(camera.Zoom),
			float(m_framebuffer.width) / float(m_framebuffer.height),
			Near, Far
		);
	glNamedBufferSubData(m_transformUB, 0, sizeof(TransformUB), &transformUniforms);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_transformUB);
	preView = transformUniforms.view;
	preProj = transformUniforms.projection;
	

	glBindFramebuffer(GL_FRAMEBUFFER, m_gbuffer.id);
	// 先将颜色缓冲清空为背景色，如果没有天空盒则将显示该颜色
	//glClearColor(scene.backgroundColor.x(), scene.backgroundColor.y(), scene.backgroundColor.z(), 1.0f);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!m_geometryPassShader.ID)
	{
		m_geometryPassShader = Shader(
			PROJECT_PATH + "/data/shaders/deferred/deferred_geo_vs.glsl",
			PROJECT_PATH + "/data/shaders/deferred/deferred_geo_fs.glsl"
		);
	}
	m_geometryPassShader.use();
	glEnable(GL_DEPTH_TEST);

	for (int i = 0; i < m_models.size(); ++i) {
		if (!m_models[i]->visible) continue;
		transformUniforms.model = m_models[i]->getToWorldMatrix();
		glNamedBufferSubData(m_transformUB, 0, sizeof(TransformUB), &transformUniforms);
		taaUniforms.preModel = m_models[i]->getPreModelMatrix();
		glNamedBufferSubData(m_taaUB, 0, sizeof(TaaUB), &taaUniforms);

		m_models[i]->setPreModelMatrix(transformUniforms.model);

		m_models[i]->draw(m_geometryPassShader);
	}

	

	// 2. 计算光照Pass

	glBindFramebuffer(GL_FRAMEBUFFER, m_interFramebuffer.id);
	
	// 录入光照信息
	ShadingUB shadingUniforms;
	const glm::vec3 eyePosition = camera.Position;
	shadingUniforms.eyePosition = glm::vec4(eyePosition, 0.0f);
	registerLight(shadingUniforms, scene);
	glNamedBufferSubData(m_shadingUB, 0, sizeof(ShadingUB), &shadingUniforms);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_shadingUB);



	if (!m_lightPassShader.ID)
	{
		m_lightPassShader = Shader(
			PROJECT_PATH + "/data/shaders/deferred/deferred_light_vs.glsl",
			PROJECT_PATH + "/data/shaders/deferred/deferred_light_fs.glsl"
		);
	}
	m_lightPassShader.use();
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	unsigned int bindIdx = 0;
	glBindTextureUnit(bindIdx++, m_gbuffer.positionTarget);
	glBindTextureUnit(bindIdx++, m_gbuffer.normalTarget);
	glBindTextureUnit(bindIdx++, m_gbuffer.colorTarget);
	glBindTextureUnit(bindIdx++, m_gbuffer.rmoTarget);
	glBindTextureUnit(bindIdx++, m_gbuffer.emissionTarget);
	m_lightPassShader.setBool("haveSkybox", scene.skybox);
	if (scene.skybox)
	{
		glBindTextureUnit(bindIdx++, m_envTexture.id);
		glBindTextureUnit(bindIdx++, m_irmapTexture.id);
	}
	else
		m_lightPassShader.setVec3("backgroundColor", scene.backgroundColor.toGlmVec());
	glBindTextureUnit(bindIdx++, m_BRDF_LUT.id);

	for (int j = 0; j < scene.NumLights; ++j)
		glBindTextureUnit(bindIdx++, scene.dirLights[j].shadowMap.id);


	glBindVertexArray(m_quadVAO);	// 屏幕VAO
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	// 转移深度信息到中介frambuffer
	m_interFramebuffer.depthStencilTarget = m_gbuffer.depthStencilTarget;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_interFramebuffer.depthStencilTarget, 0);

	// 天空盒，画完天空盒再进行TAA，不然最终结果会因为ghosting掺杂背景色（黑
	if (scene.skybox)
		drawSkybox();

	// TAA
	glBindFramebuffer(GL_FRAMEBUFFER, m_taaFrameBuffers[m_taaCurrentFrame].id);
	if (!m_taaShader.ID)
	{
		m_taaShader = Shader(
			PROJECT_PATH + "/data/shaders/taa/taa_vs.glsl",
			PROJECT_PATH + "/data/shaders/taa/taa_fs.glsl"
		);
	}
	m_taaShader.use();
	glBindTextureUnit(0, m_interFramebuffer.colorTarget);
	glBindTextureUnit(1, m_taaFrameBuffers[!m_taaCurrentFrame].colorTarget);
	glBindTextureUnit(2, m_gbuffer.velocityTarget);
	glBindTextureUnit(3, m_gbuffer.depthStencilTarget);
	m_taaShader.setFloat("ScreenWidth", ScreenWidth);
	m_taaShader.setFloat("ScreenHeight", ScreenHeight);
	m_taaShader.setInt("frameCount", frameCount);

	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	m_taaCurrentFrame ^= 1;


	//glBindFramebuffer(GL_FRAMEBUFFER, m_finalFramebuffer.id);
	//m_finalFramebuffer.colorTarget = m_taaFrameBuffers[m_taaCurrentFrame].colorTarget;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_tonemapShader.use();
	glBindTextureUnit(0, m_taaFrameBuffers[!m_taaCurrentFrame].colorTarget);
	glBindVertexArray(m_quadVAO);	// 屏幕VAO
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
