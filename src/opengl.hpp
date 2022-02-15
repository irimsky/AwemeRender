#pragma once

#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "FBO.hpp"
#include "scene_setting.hpp"
#include "meshbuffer.hpp"

#include <string>
#include <glm/mat4x4.hpp>



struct GLFWwindow;


class Renderer
{
public:
	Renderer();
	GLFWwindow* initialize(int width, int height, int maxSamples) ;
	void setup(const SceneSettings& scene) ;
	void render(GLFWwindow* window, const Camera& camera, const SceneSettings& scene);
	void renderImgui(SceneSettings& scene);
	void shutdown();

private:


	static GLuint createUniformBuffer(const void* data, size_t size);

	void loadModels(const std::string& modelName, SceneSettings& scene);
	void loadSceneHdr(const std::string& filename);
	void calcLUT();
	

	template<typename T> GLuint createUniformBuffer(const T* data = nullptr)
	{
		return createUniformBuffer(data, sizeof(T));
	}

#if _DEBUG
	static void logMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

	struct {
		float maxAnisotropy = 1.0f;
	} m_capabilities;

	FrameBuffer m_framebuffer;
	FrameBuffer m_resolveFramebuffer;

	MeshBuffer m_skybox;
	MeshBuffer m_pbrModel;

	GLuint m_quadVAO;

	Shader m_tonemapShader;
	Shader m_skyboxShader;
	Shader m_pbrShader;
	ComputeShader m_equirectToCubeShader;
	ComputeShader m_prefilterShader;
	ComputeShader m_irradianceMapShader;

	int m_EnvMapSize;
	int m_IrradianceMapSize;
	int m_BRDF_LUT_Size;

	Texture m_envTexture;
	Texture m_irmapTexture;
	Texture m_BRDF_LUT;

	Texture m_albedoTexture;
	Texture m_normalTexture;
	Texture m_metalnessTexture;
	Texture m_roughnessTexture;
	Texture m_occlusionTexture;
	Texture m_emissionTexture;
	Texture m_heightTexture;

	GLuint m_transformUB;
	GLuint m_shadingUB;
};



