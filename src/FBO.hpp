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

FrameBuffer createShadowFrameBuffer(int width, int height, Texture& shadowMap);


void resolveFramebuffer(const FrameBuffer& srcfb, const FrameBuffer& dstfb);
void deleteFrameBuffer(FrameBuffer& fb);