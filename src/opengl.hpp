#pragma once

#include "shader.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "FBO.hpp"
#include "scene_setting.hpp"
#include "meshbuffer.hpp"

#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <string>
#include <glm/mat4x4.hpp>
#include <stdexcept>


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
	void updateShadowMap(SceneSettings& scene);

private:
	static GLuint createUniformBuffer(const void* data, size_t size);

	void initShadowMap(SceneSettings& scene);
	
	void updateDirectionalLightShadowMap(DirectionalLight& light, Shader& shader);

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
	FrameBuffer m_shadowFrameBuffer;

	MeshBuffer m_skybox;
	MeshBuffer m_pbrModel;
	Model m_model;
	std::vector<Model> m_models;
	int m_selectedIdx;

	GLuint m_quadVAO;

	Shader m_tonemapShader;
	Shader m_skyboxShader;
	Shader m_pbrShader;
	ComputeShader m_equirectToCubeShader;
	ComputeShader m_prefilterShader;
	ComputeShader m_irradianceMapShader;

	Shader m_dirLightShadowShader;
	Shader m_pointLightShadowShader;

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



