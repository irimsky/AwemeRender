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


	m_dirLightShadowShader.use();
	for (int i = 0; i < scene.NumLights; ++i)
	{
		updateDirectionalLightShadowMap(scene.dirLights[i], m_dirLightShadowShader);
	}

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
		glm::mat4 model = 
			glm::translate(glm::mat4(1.0), m_models[i].position.toGlmVec()) *
			glm::eulerAngleXYZ(glm::radians(m_models[i].rotation.x()), glm::radians(m_models[i].rotation.y()), glm::radians(m_models[i].rotation.z())) *
			glm::scale(glm::mat4(1.0f), glm::vec3(m_models[i].scale));
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		if(lightPos.y == 10 || lightPos.y == -10)
			up = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), up);
		glm::mat4 lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, Near, Far);
		glm::mat4 lightMatrix = lightProjection * lightView;
		shader.setMat4("lightSpaceMatrix", lightMatrix);
		shader.setMat4("model", model);

		for (int j = 0; j < m_models[i].pbrModel.meshes.size(); ++j)
		{
			glBindVertexArray(m_models[i].pbrModel.meshes[j]->vao);
			glDrawElements(GL_TRIANGLES, m_models[i].pbrModel.meshes[j]->numElements, GL_UNSIGNED_INT, 0);
		}
	}

}