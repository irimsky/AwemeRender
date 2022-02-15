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

	Model(std::string filePath, bool detectTex=false) {
		pbrModel = createMeshBuffer(Mesh::fromFile(filePath));
		scale = 1.0f;
		position = vec3(0.0f);
		color = vec3(0.8f);
		int fileIdx = filePath.find_last_of('\\');
		name = filePath.substr((size_t)fileIdx + 1);
		if (detectTex) {
			int dotIdx = filePath.find_last_of('.');
			std::string modelPath = filePath.substr(0, (size_t)dotIdx);
			std::string format = filePath.substr((size_t)dotIdx);
			std::vector<char*> modelFiles = File::readAllFilesInDirWithExt(modelPath.substr(0, fileIdx));
			for (char* str : modelFiles)
			{
				std::string tmpStr = str;
				int dotIdx = tmpStr.find_last_of('.');
				std::string extName = tmpStr.substr(dotIdx);

				if (extName != format)
				{
					format = extName;
					break;
				}
			}

			// º”‘ÿŒ∆¿ÌÃ˘Õº
			std::cout << "Start Loading Textures:" << std::endl;
			try {
				albedoTexture = createTexture(
					Image::fromFile(modelPath + "_albedo" + format, 3),
					GL_RGB, GL_SRGB8
				);
			}
			catch (std::runtime_error) {
				std::cout << "No Albedo Texture" << std::endl;
			}

			try {
				normalTexture = createTexture(
					Image::fromFile(modelPath + "_normal" + format, 3),
					GL_RGB, GL_RGB8
				);
			}
			catch (std::runtime_error) {
				std::cout << "No Normal Texture" << std::endl;
			}
			

			try {
				metalnessTexture = createTexture(
					Image::fromFile(modelPath + "_metalness" + format, 1),
					GL_RED, GL_R8
				);
			}
			catch (std::runtime_error) {
				std::cout << "No Metal Texture" << std::endl;
			}

			try {
				roughnessTexture = createTexture(
					Image::fromFile(modelPath + "_roughness" + format, 1),
					GL_RED, GL_R8
				);
			}
			catch (std::runtime_error) {
				std::cout << "No Rough Texture" << std::endl;
			}

			try {
				occlusionTexture = createTexture(
					Image::fromFile(modelPath + "_occlusion" + format, 1),
					GL_RED, GL_R8
				);
			}
			catch (std::runtime_error) {
				std::cout << "No Occlusion Texture" << std::endl;
			}

			try {
				emissionTexture = createTexture(
					Image::fromFile(modelPath + "_emission" + format, 3),
					GL_RGB, GL_SRGB8
				);
			}
			catch (std::runtime_error) {
				std::cout << "No Emission Texture" << std::endl;
			} 

			try {
				heightTexture = createTexture(
					Image::fromFile(modelPath + "_height" + format, 1),
					GL_RED, GL_R8
				);
			}
			catch (std::runtime_error) {
				std::cout << "No Height Texture" << std::endl;
			}

		}
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

void deleteModel(Model& model);
