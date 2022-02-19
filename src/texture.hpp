#pragma once
#include <glad/glad.h>
#include "utils.hpp"
#include "image.hpp"
#include "global.hpp"

class Texture
{
public:
	Texture() : id(0) {}
	GLuint id;
	int width, height;
	int levels;

	bool exist();
};

Texture createTexture(
	GLenum target, int width, int height, GLenum internalformat,
	int levels = 0
);

Texture createTexture(
	const std::shared_ptr<class Image>& image, GLenum format, GLenum internalformat,
	int levels = 0
);

Texture createShadowMap(int width, int height);

void deleteTexture(Texture& texture);