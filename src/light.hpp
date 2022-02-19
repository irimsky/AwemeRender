#pragma once
#include "math.hpp"
#include "texture.hpp"
using namespace Math;

class Light {
public:
	vec3 radiance;
	bool enabled;
	Texture shadowMap;

	Light() : radiance(vec3(1.0f)), enabled(false) {}
};

class PointLight : public Light {
public:
	vec3 position;
	PointLight() : position(vec3(0.0f)) {}
	PointLight(const vec3& rad, const vec3& pos)
	{
		radiance = rad;
		position = pos;
	}
	PointLight(vec3&& rad, vec3&& pos)
	{
		radiance = rad;
		position = pos;
	}
};

class DirectionalLight : public Light {
public:
	vec3 direction;
	DirectionalLight() : direction(vec3(1.0f)) {}
	DirectionalLight(const vec3& rad, const vec3& dir)
	{
		radiance = rad;
		direction = dir;
	}
	DirectionalLight(vec3&& rad, vec3&& dir)
	{
		radiance = rad;
		direction = dir;
	}
};