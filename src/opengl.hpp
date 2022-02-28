#pragma once

#include "shader.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "framebuffer.hpp"
#include "scene_setting.hpp"
#include "meshbuffer.hpp"
#include "uniformbuffer.hpp"

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
	
	void deferredRender(GLFWwindow* window, const Camera& camera, const SceneSettings& scene);
	void renderImgui(SceneSettings& scene);
	void shutdown();
	void updateShadowMap(SceneSettings& scene, const Camera& camera);

private:
	// 绘制天空盒
	void drawSkybox();

	// 初始化深度图
	void initShadowMap(SceneSettings& scene);

	// 更新方向光的深度图
	void updateDirectionalLightShadowMap(DirectionalLight& light, Shader& shader);

	// 加载场景HDR
	void loadSceneHdr(const std::string& filename);

	// 计算Look Up Texture
	void calcLUT();

	// 更新AABB
	void updateAABB(const Camera& camera);

#if _DEBUG
	static void logMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

	struct {
		float maxAnisotropy = 1.0f;
	} m_capabilities;

	FrameBuffer m_framebuffer;
	FrameBuffer m_interFramebuffer;
	FrameBuffer m_shadowFrameBuffer;
	// position, normal, albedo, (roughness, metalness, occlusion), emmision
	GBuffer m_gbuffer;
	

	MeshBuffer m_skybox;
	
	typedef std::shared_ptr<Model> ModelPtr;
	std::vector<ModelPtr> m_models;

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

	Shader m_geometryPassShader;
	Shader m_lightPassShader;

	Texture m_envTexture;
	Texture m_irmapTexture;
	Texture m_BRDF_LUT;

	GLuint m_transformUB;
	TransformUB transformUniforms;
	GLuint m_shadingUB;
	ShadingUB shadingUniforms;
	GLuint m_taaUB;
	TaaUB taaUniforms;

	glm::mat4 preView, preProj;

	AABB m_boundingBox;
};


