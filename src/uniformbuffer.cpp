#include "uniformbuffer.hpp"

GLuint createUniformBuffer(const void* data, size_t size)
{
	GLuint ubo;
	glCreateBuffers(1, &ubo);
	glNamedBufferStorage(ubo, size, data, GL_DYNAMIC_STORAGE_BIT);
	return ubo;
}

void registerLight(ShadingUB& shadingUB, const SceneSettings& scene)
{
	for (int i = 0; i < SceneSettings::NumLights; ++i) {
		const DirectionalLight& light = scene.dirLights[i];
		shadingUB.lights[i].direction = glm::normalize(glm::vec4{ light.direction.toGlmVec(), 0.0f });
		if (light.enabled) {
			shadingUB.lights[i].radiance = glm::vec4{ light.radiance.toGlmVec(), 0.0f };
		}
		else {
			shadingUB.lights[i].radiance = glm::vec4{};
		}
		shadingUB.lights[i].lightSpaceMatrix = light.lightSpaceMatrix;

		const PointLight& ptLight = scene.ptLights[i];
		shadingUB.ptLights[i].position = glm::vec4(ptLight.position.toGlmVec(), 0.0f);
		if (ptLight.enabled) {
			shadingUB.ptLights[i].radiance = glm::vec4(ptLight.radiance.toGlmVec(), 0.0f);
		}
		else {
			shadingUB.ptLights[i].radiance = glm::vec4{};
		}
	}
}

