#pragma once
#include <vector>

class AABB {
public:
	float xMin, xMax, yMin, yMax, zMin, zMax;

	AABB() : xMin(0), xMax(0), yMin(0), yMax(0), zMin(0), zMax(0){}


	void merge(const AABB& aabb);
};