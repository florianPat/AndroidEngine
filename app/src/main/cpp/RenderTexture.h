#pragma once

#include "Texture.h"
#include "Sprite.h"
#include "Texture.h"
#include "Mat4x4.h"
#include "Shader.h"
#include "Vector.h"
#include <memory>

class RenderTexture
{
	GLuint renderTexture = 0;
	GLint screenTexture = 0;
	Texture texture;
	Mat4x4 orhtoProj;
	Shader* shaderSprite = nullptr;
	float windowWidth = 0;
	float windowHeight = 0;
public:
	RenderTexture() = default;
	RenderTexture(const RenderTexture& other) = delete;
	RenderTexture(RenderTexture&& other);
	RenderTexture& operator=(const RenderTexture& rhs) = delete;
	RenderTexture& operator=(RenderTexture&& rhs);
	~RenderTexture();
	bool create(uint width, uint height, Shader* shaderSprite);
	void clear();
	const Texture& getTexture() const;
	void display();
	void draw(const Sprite& sprite);
};