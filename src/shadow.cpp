#include "opengl.hpp"

void Renderer::initShadowMap(SceneSettings& scene)
{
	for (int i = 0; i < scene.NumLights; ++i)
	{
		scene.dirLights[i].shadowMap = createShadowMap(ShadowMapSize, ShadowMapSize);
		scene.ptLights[i].shadowMap = createShadowMap(ShadowMapSize, ShadowMapSize);
	}
}

void Renderer::updateShadowMap(SceneSettings& scene, const Camera& camera)
{
	glViewport(0, 0, ShadowMapSize, ShadowMapSize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFrameBuffer.id);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_DEPTH_TEST);
	// 剔除正面减少悬浮现象
	glCullFace(GL_FRONT);

	updateAABB(camera);

	m_dirLightShadowShader.use();
	for (int i = 0; i < scene.NumLights; ++i)
	{
		updateDirectionalLightShadowMap(scene.dirLights[i], m_dirLightShadowShader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
}

void Renderer::updateDirectionalLightShadowMap(DirectionalLight& light, Shader& shader) {
	attachTex2ShadowFBO(m_shadowFrameBuffer, light.shadowMap);
	glClear(GL_DEPTH_BUFFER_BIT);

	glm::vec3 center = m_boundingBox.getCenter();
	float radius = m_boundingBox.getDiagonal() * 0.5f;
	glm::vec3 lightPos = center - radius * glm::normalize(light.direction.toGlmVec());

	for (int i = 0; i < m_models.size(); ++i) {
		if (!m_models[i]->visible) continue;
		glm::mat4 model = m_models[i]->getToWorldMatrix();

		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		if((center - lightPos).y == radius || (center - lightPos).y == -radius)
			up = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), up);
		glm::mat4 lightProjection = glm::ortho(-radius, radius, -radius, radius, 0.1f, radius * 2);
		glm::mat4 lightMatrix = lightProjection * lightView;
		light.lightSpaceMatrix = lightMatrix;
		shader.setMat4("lightSpaceMatrix", lightMatrix);
		shader.setMat4("model", model);

		for (int j = 0; j < m_models[i]->pbrModel.meshes.size(); ++j)
		{
			glBindVertexArray(m_models[i]->pbrModel.meshes[j]->vao);
			glDrawElements(GL_TRIANGLES, m_models[i]->pbrModel.meshes[j]->numElements, GL_UNSIGNED_INT, 0);
		}
	}

}