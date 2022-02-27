#pragma once
#include <glm/ext/matrix_float4x4.hpp>
#include "scene_setting.hpp"

template<typename T> GLuint createUniformBuffer(const T* data = nullptr)
{
	return createUniformBuffer(data, sizeof(T));
}

GLuint createUniformBuffer(const void* data, size_t size);

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
		glm::mat4 lightSpaceMatrix;
	} lights[SceneSettings::NumLights];
	struct {
		glm::vec4 position;
		glm::vec4 radiance;
	} ptLights[SceneSettings::NumLights];
	glm::vec4 eyePosition;
};