#pragma once
#include "mesh.hpp"
#include "light.hpp"
#include <glm/mat4x4.hpp>
#include <string>


class SceneSettings
{
public:
	static const int NumLights = 3;
	DirectionalLight dirLights[NumLights];
	PointLight ptLights[NumLights];

	bool skybox = true;
	vec3 backgroundColor = vec3(0.1f);
	char* envName;
	char* preEnv;
	std::vector<char*> envNames;

	std::string objExt;
	std::string texExt;

	float objectYaw;
	float objectPitch;
	float FPS;
};
