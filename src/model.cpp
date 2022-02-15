#include "model.hpp"

bool Model::haveAlbedo()
{
	return albedoTexture.exist();
}

bool Model::haveNormal()
{
	return normalTexture.exist();
}

bool Model::haveMetalness()
{
	return metalnessTexture.exist();
}

bool Model::haveRoughness()
{
	return roughnessTexture.exist();
}

bool Model::haveOcclusion()
{
	return occlusionTexture.exist();
}

bool Model::haveEmmission()
{
	return emissionTexture.exist();
}

bool Model::haveHeight()
{
	return heightTexture.exist();
}
