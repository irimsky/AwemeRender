#include "AABB.hpp"
#include <glm/common.hpp>

void AABB::merge(const AABB& aabb)
{
	xMin = glm::min(xMin, aabb.xMin);
	xMax = glm::max(xMax, aabb.xMax);
	yMin = glm::min(yMin, aabb.yMin);
	yMax = glm::max(yMax, aabb.yMax);
	zMax = glm::max(zMax, aabb.zMax);
	zMin = glm::min(zMin, aabb.zMin);
}