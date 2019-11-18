#pragma once

#include "OGLTexture.h"
#include "Sprite.h"
#include "OGLTexture.h"
#include "Mat4x4.h"
#include "Shader.h"
#include "Vector.h"
#include "Window.h"

class OGLRenderTexture
{
	//NOTE: Same reason as in Texture!
	GLuint renderTexture = (GLuint)-1;
	GLuint screenTexture = 0;
	OGLTexture texture;
	Mat4x4 orhtoProj;

	GraphicsOGL2* gfx = nullptr;
public:
	OGLRenderTexture() = default;
	OGLRenderTexture(const OGLRenderTexture& other) = delete;
	OGLRenderTexture(OGLRenderTexture&& other);
	OGLRenderTexture& operator=(const OGLRenderTexture& rhs) = delete;
	OGLRenderTexture& operator=(OGLRenderTexture&& rhs);
	~OGLRenderTexture();
	void create(uint32_t width, uint32_t height, GraphicsOGL2* gfx);
	const OGLTexture& getTexture() const;
	void begin();
	void end();
	explicit operator bool() const;
	//NOTE: Just here because the vulkan side of thing uses it
	void changeToShaderReadMode() {}
};