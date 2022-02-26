#include "opengl.hpp"

void Renderer::deferredRender(GLFWwindow* window, const Camera& camera, const SceneSettings& scene)
{
	glViewport(0, 0, ScreenWidth, ScreenHeight);
	// 1. 几何计算Pass
	TransformUB transformUniforms;
	transformUniforms.view = camera.getViewMatrix();
	transformUniforms.projection =
		glm::perspective(
			glm::radians(camera.Zoom),
			float(m_framebuffer.width) / float(m_framebuffer.height),
			Near, Far
		);
	glNamedBufferSubData(m_transformUB, 0, sizeof(TransformUB), &transformUniforms);

	glBindFramebuffer(GL_FRAMEBUFFER, m_gbuffer.id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_transformUB);

	// 先将颜色缓冲清空为背景色，如果没有天空盒则将显示该颜色
	glClearColor(scene.backgroundColor.x(), scene.backgroundColor.y(), scene.backgroundColor.z(), 1.0f);
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
		transformUniforms.model =
			glm::translate(glm::mat4(1.0f), m_models[i]->position.toGlmVec()) *
			// TODO: 删去这一段测试用旋转代码
			glm::rotate(glm::mat4(1.0f), glm::radians(scene.objectPitch), glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(scene.objectYaw), glm::vec3(0, 1, 0)) *
			glm::eulerAngleXYZ(glm::radians(m_models[i]->rotation.x()), glm::radians(m_models[i]->rotation.y()), glm::radians(m_models[i]->rotation.z())) *
			glm::scale(glm::mat4(1.0f), glm::vec3(m_models[i]->scale));
		glNamedBufferSubData(m_transformUB, 0, sizeof(TransformUB), &transformUniforms);

		m_models[i]->draw(m_geometryPassShader);
	}

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//m_tonemapShader.use();
	//glBindTextureUnit(0, m_gbuffer.colorTarget);
	//glBindVertexArray(m_quadVAO);	// 屏幕VAO
	//glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// 2. 计算光照Pass

	glBindFramebuffer(GL_FRAMEBUFFER, m_interFramebuffer.id);
	// 转移深度信息到中介frambuffer
	m_interFramebuffer.depthStencilTarget = m_gbuffer.depthStencilTarget;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_interFramebuffer.depthStencilTarget, 0);

	// 录入光照信息
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

	glBindTextureUnit(bindIdx++, m_gbuffer.depthStencilTarget);

	glBindVertexArray(m_quadVAO);	// 屏幕VAO
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	// 天空盒
	if (scene.skybox) {
		m_skyboxShader.use();
		glDepthFunc(GL_LEQUAL);
		glBindTextureUnit(0, m_envTexture.id);

		glBindVertexArray(m_skybox.meshes[0]->vao);
		glDrawElements(GL_TRIANGLES, m_skybox.meshes[0]->numElements, GL_UNSIGNED_INT, 0);
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_tonemapShader.use();
	glBindTextureUnit(0, m_interFramebuffer.colorTarget);
	glBindVertexArray(m_quadVAO);	// 屏幕VAO
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
