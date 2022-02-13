#pragma once
#include <glad/glad.h>

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

FrameBuffer createFrameBuffer(int width, int height, int samples,
	GLenum colorFormat, GLenum depthstencilFormat
);

void resolveFramebuffer(const FrameBuffer& srcfb, const FrameBuffer& dstfb);
void deleteFrameBuffer(FrameBuffer& fb);