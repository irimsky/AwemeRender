#pragma once
#include "texture.hpp"
#include "math.hpp"
#include "meshbuffer.hpp"

#include <string>
#include <map>

using namespace Math;

class Model {
public:
	vec3 position;
	vec3 color;
	float scale;

	std::string name;

	

	MeshBuffer pbrModel;

	Texture albedoTexture;
	Texture normalTexture;
	Texture metalnessTexture;
	Texture roughnessTexture;
	Texture occlusionTexture;
	Texture emissionTexture;
	Texture heightTexture;

	Model(std::string filePath) {
		pbrModel = createMeshBuffer(Mesh::fromFile(filePath));
		scale = 1.0f;
		position = vec3(0.0f);
		color = vec3(0.8f);
		int dotIdx = filePath.find_last_of('\\');
		name = filePath.substr(dotIdx + 1);
	}
	Model() {};

	bool haveAlbedo();
	bool haveNormal();
	bool haveMetalness();
	bool haveRoughness();
	bool haveOcclusion();
	bool haveEmmission();
	bool haveHeight();

private:
	static std::map<std::string, int> nameCount;
	
};