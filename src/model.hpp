#pragma once
#include "texture.hpp"
#include "math.hpp"
#include "meshbuffer.hpp"

#include <string>
#include <unordered_map>
#include "../data/shaders/shadow/AABB.hpp"

using namespace Math;

// 模型的贴图类型
static enum class TextureType {
	Albedo, Normal, Metalness, Roughness, Occlusion, Emission, Height
};

static const std::vector<std::string> TextureTypeNames = {
	"Albedo", "Normal", "Metalness", "Roughness", "Occlusion",
	"Emission", "Height"
};

class Model {
public:
	vec3 position;
	vec3 color;
	float scale;
	bool visible;

	char name[260];
	bool isSelected;
	MeshBuffer pbrModel;

	static const int TexCount = 7;
	Texture textures[TexCount];

	Model(std::string filePath, bool detectTex=false)
		: scale(1.0f), position(vec3(0.0f)), color(vec3(0.8f)),
		isSelected(false), visible(true)
	{
		pbrModel = createMeshBuffer(Mesh::fromFiles(filePath));

		int fileIdx = filePath.find_last_of('\\');
		if(fileIdx == -1)
			fileIdx = filePath.find_last_of('/');

		int dotIdx = filePath.find_last_of('.');
		std::string tmpName = filePath.substr((size_t)fileIdx + 1, (size_t)dotIdx - fileIdx - 1);
		tmpName += "_" + std::to_string(nameCount[name]++);
		strcpy(name, tmpName.c_str());

		if (detectTex) {
			
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

			// 加载纹理贴图
			std::cout << "Start Loading Textures:" << std::endl;
			
			for (int i = 0; i < TexCount; ++i)
			{
				std::string tmp = TextureTypeNames[i];
				tmp[0] = tolower(tmp[0]);
				try {
					loadTexture(modelPath + "_" + tmp + format, (TextureType)i);
				}
				catch (std::runtime_error) {
					std::cout << "No " + TextureTypeNames[i] +" Texture" << std::endl;
				}
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
	bool haveTexture(TextureType type);

	AABB getBoundingBox(glm::mat4 toWorldMatrix);

	void loadTexture(std::string filePath, TextureType type);

	


protected:
	static std::unordered_map<std::string, int> nameCount;
	/*void loadAlbedoTexture(std::string filePath);
	void loadNormalTexture(std::string filePath);
	void loadMetalnessTexture(std::string filePath);
	void loadRoughnessTexture(std::string filePath);
	void loadOcclusionTexture(std::string filePath);
	void loadEmmissionTexture(std::string filePath);
	void loadHeightTexture(std::string filePath);*/
};

void deleteModel(Model& model);
