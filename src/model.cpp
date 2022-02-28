#include "model.hpp"

std::unordered_map<std::string, int> Model::nameCount;

bool Model::haveTexture(TextureType type)
{
	return textures[(int)type].exist();
}

void Model::loadTexture(std::string filePath, TextureType type)
{
	if (type == TextureType::Albedo)
	{
		textures[(int)type] = createTexture(
			Image::fromFile(filePath, 3),
			GL_RGB, GL_SRGB8
		);
	}

	else if (type == TextureType::Normal)
	{
		textures[(int)type] = createTexture(
			Image::fromFile(filePath, 3),
			GL_RGB, GL_RGB8
		);
	}

	else if (type == TextureType::Metalness)
	{
		textures[(int)type] = createTexture(
			Image::fromFile(filePath, 1),
			GL_RED, GL_R8
		);
	}

	else if (type == TextureType::Roughness)
	{
		textures[(int)type] = createTexture(
			Image::fromFile(filePath, 1),
			GL_RED, GL_R8
		);
	}

	else if (type == TextureType::Occlusion)
	{
		textures[(int)type] = createTexture(
			Image::fromFile(filePath, 1),
			GL_RED, GL_R8
		);
	}

	else if (type == TextureType::Emission)
	{
		textures[(int)type] = createTexture(
			Image::fromFile(filePath, 3),
			GL_RGB, GL_SRGB8
		);
	}

	else if (type == TextureType::Height)
	{
		textures[(int)type] = createTexture(
			Image::fromFile(filePath, 1),
			GL_RED, GL_R8
		);
	}
}

glm::mat4 Model::getToWorldMatrix()
{
	return
	glm::translate(glm::mat4(1.0f), position.toGlmVec()) *
	glm::eulerAngleXYZ(glm::radians(rotation.x()), glm::radians(rotation.y()), glm::radians(rotation.z()))*
	glm::scale(glm::mat4(1.0f), glm::vec3(scale));
}

void Model::setPreModelMatrix(const glm::mat4& mat)
{
	preModelMat = mat;
}

glm::mat4 Model::getPreModelMatrix()
{
	return preModelMat;
}

void Model::draw()
{
	for (int i = 0; i < pbrModel.meshes.size(); ++i)
	{
		glBindVertexArray(pbrModel.meshes[i]->vao);

		if(modelType == ModelType::Sphere) 
			glDrawElements(GL_TRIANGLE_STRIP, pbrModel.meshes[i]->numElements, GL_UNSIGNED_INT, 0);
		else if(modelType == ModelType::Cube)
			glDrawArrays(GL_TRIANGLES, 0, 36);
		else if (modelType == ModelType::Plane) {
			glDisable(GL_CULL_FACE);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glEnable(GL_CULL_FACE);
		}
		else
			glDrawElements(GL_TRIANGLES, pbrModel.meshes[i]->numElements, GL_UNSIGNED_INT, 0);
		
		glBindVertexArray(0);
	}
}

void Model::draw(Shader& shader)
{
	for (int j = 0; j < Model::TexCount; ++j)
	{
		std::string typeName = TextureTypeNames[j];
		if (haveTexture((TextureType)j))
		{
			shader.setBool("have" + typeName, true);
			glBindTextureUnit(j, textures[j].id);
		}
		else
		{
			shader.setBool("have" + typeName, false);
			if (j == (int)TextureType::Albedo)
				shader.setVec3("commonColor", color.toGlmVec());
		}
	}

	for (int i = 0; i < pbrModel.meshes.size(); ++i)
	{
		glBindVertexArray(pbrModel.meshes[i]->vao);

		if (modelType == ModelType::Sphere)
			glDrawElements(GL_TRIANGLE_STRIP, pbrModel.meshes[i]->numElements, GL_UNSIGNED_INT, 0);
		else if (modelType == ModelType::Cube)
			glDrawArrays(GL_TRIANGLES, 0, 36);
		else if (modelType == ModelType::Plane) {
			glDisable(GL_CULL_FACE);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glEnable(GL_CULL_FACE);
		}
		else
			glDrawElements(GL_TRIANGLES, pbrModel.meshes[i]->numElements, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
}

Model* Model::createSphere()
{
	Model* sphere = new Model();
	std::string tmpName = "sphere";
	tmpName += "_" + std::to_string(nameCount[tmpName]++);
	strcpy(sphere->name, tmpName.c_str());
	sphere->pbrModel = createMeshBuffer(Mesh::createSphereMesh(), true);
	sphere->modelType = Model::ModelType::Sphere;
	return sphere;
}

Model* Model::createCube()
{
	Model* cube = new Model();
	std::string tmpName = "cube";
	tmpName += "_" + std::to_string(nameCount[tmpName]++);
	strcpy(cube->name, tmpName.c_str());
	cube->pbrModel = createMeshBuffer(Mesh::createCubeMesh(), true);
	cube->modelType = Model::ModelType::Cube;
	return cube;
}

Model* Model::createPlane()
{
	Model* plane = new Model();
	std::string tmpName = "plane";
	tmpName += "_" + std::to_string(nameCount[tmpName]++);
	strcpy(plane->name, tmpName.c_str());
	plane->pbrModel = createMeshBuffer(Mesh::createPlaneMesh(), true);
	plane->modelType = Model::ModelType::Plane;
	return plane;
}

void deleteModel(Model& model)
{
	deleteMeshBuffer(model.pbrModel);
	for (int i = 0; i < Model::TexCount; ++i)
		deleteTexture(model.textures[i]);

	std::memset(&model, 0, sizeof(Model));
}

void deleteModel(std::shared_ptr<Model> model)
{
	deleteMeshBuffer(model->pbrModel);
	for (int i = 0; i < Model::TexCount; ++i)
		deleteTexture(model->textures[i]);

	model.reset();
}


