#pragma once
#include "texture.hpp"
#include "math.hpp"
#include "meshbuffer.hpp"
#include "AABB.hpp"
#include "shader.hpp"

#include <string>
#include <unordered_map>


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
	vec3 rotation;
	float scale;
	bool visible;

	char name[260];
	bool isSelected;

	MeshBuffer pbrModel;
	enum class ModelType {
		ImportModel,
		Sphere, Cube, Plane
	} modelType = Model::ModelType::ImportModel;

	static const int TexCount = 7;
	Texture textures[TexCount];

	Model(std::string filePath, bool detectTex=false)
		: scale(1.0f), position(vec3(0.0f)), color(vec3(0.8f)), rotation(vec3(0.0f)),
		isSelected(false), visible(true)
	{
		pbrModel = createMeshBuffer(Mesh::fromFiles(filePath));

		int fileIdx = filePath.find_last_of('\\');
		if(fileIdx == -1)
			fileIdx = filePath.find_last_of('/');

		int dotIdx = filePath.find_last_of('.');
		std::string tmpName = filePath.substr((size_t)fileIdx + 1, (size_t)dotIdx - fileIdx - 1);
		tmpName += "_" + std::to_string(nameCount[tmpName]++);
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
	Model() 
		: scale(1.0f), position(vec3(0.0f)), color(vec3(0.8f)), rotation(vec3(0.0f)),
		isSelected(false), visible(true) 
	{};
	static Model* createSphere();
	static Model* createCube();
	static Model* createPlane();

	void loadTexture(std::string filePath, TextureType type);
	bool haveTexture(TextureType type);

	glm::mat4 getToWorldMatrix();
	void setPreModelMatrix(const glm::mat4& mat);
	glm::mat4 getPreModelMatrix();

	void draw();
	void draw(Shader& shader);

	AABB getBoundingBox();


protected:
	glm::mat4 preModelMat;
	static std::unordered_map<std::string, int> nameCount;
};



void deleteModel(Model& model);

void deleteModel(std::shared_ptr<Model> model);
