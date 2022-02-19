#include "opengl.hpp"

void Renderer::initShadowMap(SceneSettings& scene)
{
	for (int i = 0; i < scene.NumLights; ++i)
	{
		scene.dirLights[i].shadowMap = createShadowMap(ShadowMapSize, ShadowMapSize);
		scene.ptLights[i].shadowMap = createShadowMap(ShadowMapSize, ShadowMapSize);
	}
}

void updateShadowMap(SceneSettings& scene)
{
	 
}

void updateDirectionalLightShadowMap(DirectionalLight& light) {

}