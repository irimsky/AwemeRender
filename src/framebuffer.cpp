#include "framebuffer.hpp"


#include <vector>
#include <stdexcept>
#include <string>
#include <cassert>
#include <memory>


// 创建用RBO做附件的FBO
FrameBuffer createFrameBufferWithRBO(int width, int height, int samples,
	GLenum colorFormat, GLenum depthstencilFormat
)
{
	FrameBuffer fb;
	fb.width = width;
	fb.height = height;
	fb.samples = samples;

	glCreateFramebuffers(1, &fb.id);

	// 为Framebuffer创建Renderbuffer
	// 可用于分配和存储颜色，深度或模板值，并可用作帧缓冲区对象中的颜色，深度或模板附件
	if (colorFormat != GL_NONE) {
		if (samples > 0) {
			glCreateRenderbuffers(1, &fb.colorTarget);
			glNamedRenderbufferStorageMultisample(fb.colorTarget, samples, colorFormat, width, height);
			glNamedFramebufferRenderbuffer(fb.id, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fb.colorTarget);
		}
		else {
			glCreateTextures(GL_TEXTURE_2D, 1, &fb.colorTarget);
			glTextureStorage2D(fb.colorTarget, 1, colorFormat, width, height);
			glNamedFramebufferTexture(fb.id, GL_COLOR_ATTACHMENT0, fb.colorTarget, 0);
		}
	}
	// 为Renderbuffer指定深度/模板格式
	if (depthstencilFormat != GL_NONE) {
		glCreateRenderbuffers(1, &fb.depthStencilTarget);
		if (samples > 0) {
			glNamedRenderbufferStorageMultisample(fb.depthStencilTarget, samples, depthstencilFormat, width, height);
		}
		else {
			glNamedRenderbufferStorage(fb.depthStencilTarget, depthstencilFormat, width, height);
		}
		glNamedFramebufferRenderbuffer(fb.id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.depthStencilTarget);
	}

	// 检查Framebuffer完成状态
	GLenum status = glCheckNamedFramebufferStatus(fb.id, GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer创建失败: " + std::to_string(status));
	}
	return fb;
}

void resolveFramebuffer(const FrameBuffer& srcfb, const FrameBuffer& dstfb)
{
	if (srcfb.id == dstfb.id) {
		return;
	}

	std::vector<GLenum> attachments;
	if (srcfb.colorTarget) {
		attachments.push_back(GL_COLOR_ATTACHMENT0);
	}
	if (srcfb.depthStencilTarget) {
		attachments.push_back(GL_DEPTH_STENCIL_ATTACHMENT);
	}
	assert(attachments.size() > 0);

	glBlitNamedFramebuffer(
		srcfb.id, dstfb.id, 0, 0,
		srcfb.width, srcfb.height, 0, 0,
		dstfb.width, dstfb.height, GL_COLOR_BUFFER_BIT, GL_NEAREST
	);
	glInvalidateNamedFramebufferData(srcfb.id, (GLsizei)attachments.size(), &attachments[0]);
}

void deleteFrameBuffer(FrameBuffer& fb)
{
	if (fb.id) {
		glDeleteFramebuffers(1, &fb.id);
	}
	if (fb.colorTarget) {
		if (fb.samples == 0) {
			glDeleteTextures(1, &fb.colorTarget);
		}
		else {
			glDeleteRenderbuffers(1, &fb.colorTarget);
		}
	}
	if (fb.depthStencilTarget) {
		glDeleteRenderbuffers(1, &fb.depthStencilTarget);
	}
	std::memset(&fb, 0, sizeof(FrameBuffer));
}

// 创建一个用于ShadowMap的FBO
FrameBuffer createShadowFrameBuffer(int width, int height)
{
	FrameBuffer fb;
	glCreateFramebuffers(1, &fb.id);
	fb.width = width;
	fb.height = height;
	fb.samples = 0;
	return fb;
}

// 把阴影贴图附加到FBO的深度附件上
void attachTex2ShadowFBO(FrameBuffer& fb, Texture& shadowMap)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fb.id);
	glBindTexture(GL_TEXTURE_2D, shadowMap.id);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap.id, 0
	);
	// 显式告诉OpenGL我们不用任何颜色数据进行渲染
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindTexture(GL_TEXTURE_2D, 0);
}
