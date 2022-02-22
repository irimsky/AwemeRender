#include "meshbuffer.hpp"

MeshBuffer createMeshBuffer(const std::shared_ptr<class Mesh>& mesh, bool builded)
{
	MeshBuffer buffer;
	buffer.meshes.push_back(mesh);
	if (builded) return buffer;

	buffer.meshes[0]->numElements = static_cast<GLuint>(mesh->faces().size()) * 3;
	const size_t vertexDataSize = mesh->vertices().size() * sizeof(Mesh::Vertex);
	const size_t indexDataSize = mesh->faces().size() * sizeof(Mesh::Face);

	glCreateBuffers(1, &buffer.meshes[0]->vbo);
	glNamedBufferStorage(buffer.meshes[0]->vbo, vertexDataSize, reinterpret_cast<const void*>(&buffer.meshes[0]->vertices()[0]), 0);
	glCreateBuffers(1, &buffer.meshes[0]->ibo);
	glNamedBufferStorage(buffer.meshes[0]->ibo, indexDataSize, reinterpret_cast<const void*>(&buffer.meshes[0]->faces()[0]), 0);

	glCreateVertexArrays(1, &buffer.meshes[0]->vao);
	glVertexArrayElementBuffer(buffer.meshes[0]->vao, buffer.meshes[0]->ibo);
	for (int i = 0; i < Mesh::NumAttributes; ++i) {
		glVertexArrayVertexBuffer(buffer.meshes[0]->vao, i, buffer.meshes[0]->vbo, i * sizeof(glm::vec3), sizeof(Mesh::Vertex));
		glEnableVertexArrayAttrib(buffer.meshes[0]->vao, i);
		glVertexArrayAttribFormat(buffer.meshes[0]->vao, i, i == 2 ? 2 : 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(buffer.meshes[0]->vao, i, i);
	}
	return buffer;
}

MeshBuffer createMeshBuffer(const std::vector<MeshPtr>& meshes)
{
	MeshBuffer buffer;
	buffer.meshes = meshes;
	for (int i = 0; i < meshes.size(); ++i) 
	{
		const size_t vertexDataSize = meshes[i]->vertices().size() * sizeof(Mesh::Vertex);
		const size_t indexDataSize = meshes[i]->faces().size() * sizeof(Mesh::Face);
		
		meshes[i]->numElements = static_cast<GLuint>(meshes[i]->faces().size()) * 3;
		glCreateBuffers(1, &meshes[i]->vbo);
		glNamedBufferStorage(
			meshes[i]->vbo, vertexDataSize,
			reinterpret_cast<const void*>(&meshes[i]->vertices()[0]), 0
		);
		
		glCreateBuffers(1, &meshes[i]->ibo);
		glNamedBufferStorage(
			meshes[i]->ibo, indexDataSize,
			reinterpret_cast<const void*>(&meshes[i]->faces()[0]), 0
		);

		glCreateVertexArrays(1, &meshes[i]->vao);
		glVertexArrayElementBuffer(meshes[i]->vao, meshes[i]->ibo);
		for (int i = 0; i < Mesh::NumAttributes; ++i) {
			glVertexArrayVertexBuffer(meshes[i]->vao, i, meshes[i]->vbo, i * sizeof(glm::vec3), sizeof(Mesh::Vertex));
			glEnableVertexArrayAttrib(meshes[i]->vao, i);
			glVertexArrayAttribFormat(meshes[i]->vao, i, i == 2 ? 2 : 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(meshes[i]->vao, i, i);
		}
	}
	
	return buffer;
}

MeshBuffer createMeshBuffer(std::vector<MeshPtr>&& meshes)
{
	MeshBuffer buffer;
	buffer.meshes = meshes;
	for (int i = 0; i < meshes.size(); ++i)
	{
		const size_t vertexDataSize = meshes[i]->vertices().size() * sizeof(Mesh::Vertex);
		const size_t indexDataSize = meshes[i]->faces().size() * sizeof(Mesh::Face);

		meshes[i]->numElements = static_cast<GLuint>(meshes[i]->faces().size()) * 3;
		glCreateBuffers(1, &meshes[i]->vbo);
		glNamedBufferStorage(
			meshes[i]->vbo, vertexDataSize,
			reinterpret_cast<const void*>(&meshes[i]->vertices()[0]), 0
		);

		glCreateBuffers(1, &meshes[i]->ibo);
		glNamedBufferStorage(
			meshes[i]->ibo, indexDataSize,
			reinterpret_cast<const void*>(&meshes[i]->faces()[0]), 0
		);

		glCreateVertexArrays(1, &meshes[i]->vao);
		glVertexArrayElementBuffer(meshes[i]->vao, meshes[i]->ibo);
		for (int j = 0; j < Mesh::NumAttributes; ++j) {
			glVertexArrayVertexBuffer(meshes[i]->vao, j, meshes[i]->vbo, j * sizeof(glm::vec3), sizeof(Mesh::Vertex));
			glEnableVertexArrayAttrib(meshes[i]->vao, j);
			glVertexArrayAttribFormat(meshes[i]->vao, j, j == 2 ? 2 : 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(meshes[i]->vao, j, j);
		}
	}

	return buffer;
}

void deleteMeshBuffer(MeshBuffer& buffer)
{
	if (buffer.vao) {
		glDeleteVertexArrays(1, &buffer.vao);
	}
	if (buffer.vbo) {
		glDeleteBuffers(1, &buffer.vbo);
	}
	if (buffer.ibo) {
		glDeleteBuffers(1, &buffer.ibo);
	}
	std::memset(&buffer, 0, sizeof(MeshBuffer));
}