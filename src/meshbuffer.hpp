#pragma once
#include <glad/glad.h>

#include "mesh.hpp"


class MeshBuffer
{
public:
	MeshBuffer() : vbo(0), ibo(0), vao(0) {}
	GLuint vbo, ibo, vao;
	GLuint numElements;


};

MeshBuffer createMeshBuffer(const std::shared_ptr<class Mesh>& mesh);
void deleteMeshBuffer(MeshBuffer& buffer);