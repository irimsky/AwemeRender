#include "model.hpp"

std::unordered_map<std::string, int> Model::nameCount;

//bool Model::haveAlbedo()
//{
//	return albedoTexture.exist();
//}
//
//bool Model::haveNormal()
//{
//	return normalTexture.exist();
//}
//
//bool Model::haveMetalness()
//{
//	return metalnessTexture.exist();
//}
//
//bool Model::haveRoughness()
//{
//	return roughnessTexture.exist();
//}
//
//bool Model::haveOcclusion()
//{
//	return occlusionTexture.exist();
//}
//
//bool Model::haveEmmission()
//{
//	return emissionTexture.exist();
//}
//
//bool Model::haveHeight()
//{
//	return heightTexture.exist();
//}

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

AABB Model::getBoundingBox(glm::mat4 toWorldMatrix)
{
	//for(auto v: pbrModel.)
	return AABB();
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

void deleteModel(Model& model)
{
	deleteMeshBuffer(model.pbrModel);
	for (int i = 0; i < Model::TexCount; ++i)
		deleteTexture(model.textures[i]);

	std::memset(&model, 0, sizeof(Model));
}


