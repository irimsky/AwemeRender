#pragma once
#include <glad/glad.h>
#include "utils.hpp"
#include "image.hpp"

class Texture
{
public:
	Texture() : id(0) {}
	GLuint id;
	int width, height;
	int levels;
};

Texture createTexture(
	GLenum target, int width, int height, GLenum internalformat,
	int levels = 0, float maxAnisotropy = 1.0f
);

Texture createTexture(
	const std::shared_ptr<class Image>& image, GLenum format, GLenum internalformat,
	int levels = 0
);

void deleteTexture(Texture& texture);