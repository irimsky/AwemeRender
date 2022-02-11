#pragma once
#include "math.hpp"

class Light {
public:
	Math::vec3 radiance;
	bool enabled;
};

class PointLight : public Light {
public:
	Math::vec3 position;
};

class DirectionalLight : public Light {
public:
	Math::vec3 direction;
};