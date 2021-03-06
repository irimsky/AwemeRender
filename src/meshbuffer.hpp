#pragma once
#include <glad/glad.h>

#include "mesh.hpp"

typedef std::shared_ptr<Mesh> MeshPtr;

class MeshBuffer
{
public:
	MeshBuffer() : vbo(0), ibo(0), vao(0) {}
	GLuint vbo, ibo, vao;
	GLuint numElements;
	

	std::vector<MeshPtr> meshes;
};

MeshBuffer createMeshBuffer(const std::shared_ptr<class Mesh>& mesh, bool builded = false);
MeshBuffer createMeshBuffer(const std::vector<MeshPtr>& meshes);
MeshBuffer createMeshBuffer(std::vector<MeshPtr>&& meshes);
void deleteMeshBuffer(MeshBuffer& buffer);