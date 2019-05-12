#pragma once

#include "Texture.h"
#include "Sprite.h"
#include "Texture.h"
#include "Mat4x4.h"
#include "Shader.h"
#include "Vector.h"
#include <memory>
#include "Window.h"

class RenderTexture
{
	//NOTE: Same reason as in Texture!
	GLuint renderTexture = -1;
	GLuint screenTexture = 0;
	Texture texture;
	Mat4x4 orhtoProj;
public:
	RenderTexture() = default;
	RenderTexture(const RenderTexture& other) = delete;
	RenderTexture(RenderTexture&& other);
	RenderTexture& operator=(const RenderTexture& rhs) = delete;
	RenderTexture& operator=(RenderTexture&& rhs);
	~RenderTexture();
	bool create(uint width, uint height);
	const Texture& getTexture() const;
	void begin(Graphics& gfx);
	void end(Graphics& gfx);
	explicit operator bool() const;
};