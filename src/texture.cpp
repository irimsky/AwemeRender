#include "texture.hpp"

Texture createTexture(GLenum target, int width, int height, GLenum internalformat,
	int levels, float maxAnisotropy)
{
	Texture texture;
	texture.width = width;
	texture.height = height;
	texture.levels = (levels > 0) ? levels : Utility::numMipmapLevels(width, height);

	glCreateTextures(target, 1, &texture.id);
	glTextureStorage2D(texture.id, texture.levels, internalformat, width, height);
	glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, texture.levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameterf(texture.id, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
	return texture;
}

Texture createTexture(const std::shared_ptr<class Image>& image, GLenum format,
	GLenum internalformat, int levels)
{
	Texture texture = createTexture(GL_TEXTURE_2D, image->width(), image->height(), internalformat, levels);
	if (image->isHDR()) {
		glTextureSubImage2D(texture.id, 0, 0, 0, texture.width, texture.height, format, GL_FLOAT, image->pixels<float>());
	}
	else {
		glTextureSubImage2D(texture.id, 0, 0, 0, texture.width, texture.height, format, GL_UNSIGNED_BYTE, image->pixels<unsigned char>());
	}

	if (texture.levels > 1) {
		glGenerateTextureMipmap(texture.id);
	}
	return texture;
}

void deleteTexture(Texture& texture)
{
	glDeleteTextures(1, &texture.id);
	std::memset(&texture, 0, sizeof(Texture));
}