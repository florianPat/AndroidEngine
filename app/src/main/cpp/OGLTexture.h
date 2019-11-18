#pragma once

#include <GLES2/gl2.h>
#include "android_native_app_glue.h"
#include "String.h"
#include "Vector2.h"
#include "AssetLoader.h"

class OGLTexture
{
private:
	static constexpr bool pixeld = false;

	//NOTE: Done because if you default construct a texture, then copy construct one, the destructor
	// could delete it if it is the 0st one
	GLuint texture = (GLuint)-1;
	uint32_t width = 0;
	uint32_t height = 0;
public:
	void reloadFromFile(const String& filename);
public:
	OGLTexture() = default;
	OGLTexture(const String& filename);
	OGLTexture(const void* buffer, uint32_t width, uint32_t height, GLint internalFormat = GL_RGBA);
	OGLTexture(const OGLTexture& other) = delete;
	OGLTexture(OGLTexture&& other);
	OGLTexture& operator=(const OGLTexture& rhs) = delete;
	OGLTexture& operator=(OGLTexture&& rhs);
	~OGLTexture();
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	uint64_t getSize() const { return (width * height * sizeof(int32_t) + sizeof(OGLTexture)); }
	explicit operator bool() const;
	void bind(int32_t slot = 0) const;
	GLuint getTextureId() const { return texture; }
private:
	void createGpuTexture(const void* buffer, int32_t width, int32_t height, GLint internalFormat);
};