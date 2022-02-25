#pragma once
#include <glad/glad.h>
#include "texture.hpp"

class FrameBuffer
{
public:
	FrameBuffer() : id(0), colorTarget(0), depthStencilTarget(0) {}
	GLuint id;
	GLuint colorTarget;
	GLuint depthStencilTarget;
	int width, height;
	int samples;
};

FrameBuffer createFrameBufferWithRBO(int width, int height, int samples,
	GLenum colorFormat, GLenum depthstencilFormat
);

void resolveFramebuffer(const FrameBuffer& srcfb, const FrameBuffer& dstfb);
void deleteFrameBuffer(FrameBuffer& fb);

FrameBuffer createShadowFrameBuffer(int width, int height);
void attachTex2ShadowFBO(FrameBuffer& fb, Texture& shadowMap);

class GBuffer : public FrameBuffer
{
public:
	GLuint positionTarget, normalTarget, 
		   rmoTarget,	// rough,metal,occlusion 三合一
		   emissionTarget;
};

GBuffer createGBuffer(int width, int height);
void deleteFrameBuffer(GBuffer& fb);