#include "model.hpp"

std::unordered_map<std::string, int> Model::nameCount;

bool Model::haveTexture(TextureType type)
{
	return textures[(int)type].exist();

	//if (type == TextureType::Albedo)
	//{
	//	return haveAlbedo();
	//}

	//else if (type == TextureType::Normal)
	//{
	//	return haveNormal();
	//}

	//else if (type == TextureType::Metalness)
	//{
	//	return haveMetalness();
	//}

	//else if (type == TextureType::Roughness)
	//{
	//	return haveRoughness();
	//}

	//else if (type == TextureType::Occlusion)
	//{
	//	return haveOcclusion();
	//}

	//else if (type == TextureType::Emission)
	//{
	//	return haveEmmission();
	//}

	//else if (type == TextureType::Height)
	//{
	//	return haveHeight();
	//}
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
	return glm::translate(glm::mat4(1.0f), position.toGlmVec()) *
	glm::eulerAngleXYZ(glm::radians(rotation.x()), glm::radians(rotation.y()), glm::radians(rotation.z()))*
	glm::scale(glm::mat4(1.0f), glm::vec3(scale));
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


