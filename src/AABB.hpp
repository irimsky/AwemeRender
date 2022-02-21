#pragma once
#include "mesh.hpp"
#include <vector>
#include <glm/common.hpp>

class AABB {
public:
	float xMin, xMax, yMin, yMax, zMin, zMax;

	AABB() : xMin(0), xMax(0), yMin(0), yMax(0), zMin(0), zMax(0) {}


	void merge(const AABB& aabb);
	void mergeVertex(const Mesh::Vertex& v, const glm::mat4& toWorldMatrix);
	void mergeVertex(const glm::vec3& v);
	void reset();
	void set(glm::vec3 minn, glm::vec3 maxx);
	float getDiagonal();
	glm::vec3 getCenter();
};