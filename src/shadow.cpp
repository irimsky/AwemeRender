#include "opengl.hpp"

void Renderer::initShadowMap(SceneSettings& scene)
{
	for (int i = 0; i < scene.NumLights; ++i)
	{
		scene.dirLights[i].shadowMap = createShadowMap(ShadowMapSize, ShadowMapSize);
		scene.ptLights[i].shadowMap = createShadowMap(ShadowMapSize, ShadowMapSize);
	}
}

void Renderer::updateShadowMap(SceneSettings& scene)
{
	glViewport(0, 0, ShadowMapSize, ShadowMapSize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFrameBuffer.id);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_DEPTH_TEST);

	Shader dirLightShadowShader = Shader(
			PROJECT_PATH + "/data/shaders/shadow/directionalDepth_vs.glsl",
			PROJECT_PATH + "/data/shaders/shadow/directionalDepth_fs.glsl"
	);

	dirLightShadowShader.use();
	for (int i = 0; i < scene.NumLights; ++i)
	{
		updateDirectionalLightShadowMap(scene.dirLights[i], dirLightShadowShader);
	}
	dirLightShadowShader.deleteProgram();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::updateDirectionalLightShadowMap(DirectionalLight& light, Shader& shader) {
	attachTex2ShadowFBO(m_shadowFrameBuffer, light.shadowMap);
	glClear(GL_DEPTH_BUFFER_BIT);

	glm::vec3 lightPos = glm::normalize(light.direction.toGlmVec());
	lightPos *= -10;

	for (int i = 0; i < m_models.size(); ++i) {
		if (!m_models[i].visible) continue;
		glm::mat4 model 
			= glm::translate(glm::mat4(1.0), m_models[i].position.toGlmVec())
			* glm::scale(glm::mat4(1.0f), glm::vec3(m_models[i].scale));
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		if(lightPos.y == 10 || lightPos.y == -10)
			up = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), up);
		glm::mat4 lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, Near, Far);
		glm::mat4 lightMatrix = lightProjection * lightView;
		shader.setMat4("lightSpaceMatrix", lightMatrix);
		shader.setMat4("model", model);

		glBindVertexArray(m_models[i].pbrModel.vao);
		glDrawElements(GL_TRIANGLES, m_models[i].pbrModel.numElements, GL_UNSIGNED_INT, 0);
	}

}