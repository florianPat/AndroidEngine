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
	//NOTE: This could be a source of bugs! See Texture for details
	GLuint renderTexture = 0;
	GLuint screenTexture = 0;
	Texture texture;
	Mat4x4 orhtoProj;
	Shader* shaderSprite = nullptr;
	int windowWidth = 0;
	int windowHeight = 0;
public:
	RenderTexture() = default;
	RenderTexture(const RenderTexture& other) = delete;
	RenderTexture(RenderTexture&& other);
	RenderTexture& operator=(const RenderTexture& rhs) = delete;
	RenderTexture& operator=(RenderTexture&& rhs);
	~RenderTexture();
	bool create(uint width, uint height, Shader* shaderSprite);
	void clear(const Color& color = Color());
	const Texture& getTexture() const;
	void display();
	void draw(const Sprite& sprite);
	explicit operator bool() const;
};