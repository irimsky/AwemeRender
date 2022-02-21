#include "AABB.hpp"
#include "model.hpp"
#include "opengl.hpp"

void AABB::merge(const AABB& aabb)
{
	xMin = glm::min(xMin, aabb.xMin);
	xMax = glm::max(xMax, aabb.xMax);
	yMin = glm::min(yMin, aabb.yMin);
	yMax = glm::max(yMax, aabb.yMax);
	zMax = glm::max(zMax, aabb.zMax);
	zMin = glm::min(zMin, aabb.zMin);
}

void AABB::mergeVertex(const Mesh::Vertex& v, const glm::mat4& toWorldMatrix)
{
	glm::vec4 pos = {v.position.x, v.position.y, v.position.z, 0};
	pos = toWorldMatrix * pos;
	xMin = glm::min(xMin, pos.x);
	xMax = glm::max(xMax, pos.x);
	yMin = glm::min(yMin, pos.y);
	yMax = glm::max(yMax, pos.y);
	zMax = glm::max(zMax, pos.z);
	zMin = glm::min(zMin, pos.z);
}

void AABB::mergeVertex(const glm::vec3& v)
{
	xMin = glm::min(xMin, v.x);
	xMax = glm::max(xMax, v.x);
	yMin = glm::min(yMin, v.y);
	yMax = glm::max(yMax, v.y);
	zMax = glm::max(zMax, v.z);
	zMin = glm::min(zMin, v.z);
}

void AABB::reset()
{
	xMin, xMax, yMin, yMax, zMin, zMax = 0;
}

void AABB::set(glm::vec3 minn, glm::vec3 maxx)
{
	xMin = minn.x;
	yMin = minn.y;
	zMin = minn.z;
	xMax = maxx.x;
	yMax = maxx.y;
	zMax = maxx.z;
}

float AABB::getDiagonal()
{
	return sqrt(
		(xMax-xMin) * (xMax - xMin) + (yMax - yMin) * (yMax - yMin) + (zMax - zMin) * (zMax - zMin)
	);
}

glm::vec3 AABB::getCenter()
{
	return glm::vec3((xMin+xMax)/2, (yMin + yMax) / 2, (zMin + zMax) / 2);
}

AABB Model::getBoundingBox()
{
	AABB res;
	glm::mat4 toWorldMatrix = getToWorldMatrix();
	for (MeshPtr m : pbrModel.meshes)
	{
		for (const Mesh::Vertex& v : m->vertices())
		{
			res.mergeVertex(v, toWorldMatrix);
		}
	}
	glm::vec4 minn = { res.xMin, res.yMin, res.zMin, 1.0 };
	glm::vec4 maxx = { res.xMax, res.yMax, res.zMax, 1.0 };
	res.set(minn, maxx);
	return res;
}

void Renderer::updateAABB(const Camera& camera) {
	m_boundingBox.reset();
	glm::mat4 localToWorldMatrix = camera.getLocalToWorldMatrix();
	float T = Far * glm::tan(glm::radians(camera.Zoom) / 2);
	float L = T * 16.0f / 9.0f;
	m_boundingBox.mergeVertex(localToWorldMatrix * glm::vec4(-L, T, -Near, 1));
	m_boundingBox.mergeVertex(localToWorldMatrix * glm::vec4(L, -T, -Near, 1));
	m_boundingBox.mergeVertex(localToWorldMatrix * glm::vec4(-L, -T, -Near, 1));
	m_boundingBox.mergeVertex(localToWorldMatrix * glm::vec4(L, T, -Near, 1));

	m_boundingBox.mergeVertex(localToWorldMatrix * glm::vec4(-L, T, -Far, 1));
	m_boundingBox.mergeVertex(localToWorldMatrix * glm::vec4(L, -T, -Far, 1));
	m_boundingBox.mergeVertex(localToWorldMatrix * glm::vec4(-L, -T, -Far, 1));
	m_boundingBox.mergeVertex(localToWorldMatrix * glm::vec4(L, T, -Far, 1));
}