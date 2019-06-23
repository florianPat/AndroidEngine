#pragma once

#include <GLES2/gl2.h>
#include "android_native_app_glue.h"
#include "String.h"
#include "Vector2.h"
#include "AssetLoader.h"

class Texture
{
private:
	static constexpr bool pixeld = false;

	//NOTE: Done because if you default construct a texture, then copy construct one, the destructor
	// could delete it if it is the 0st one
	GLuint texture = -1;
	int32_t width = 0;
	int32_t height = 0;
public:
	bool loadFromFile(const String& filename);
	bool reloadFromFile(const String& filename);
public:
	Texture() = default;
	Texture(const void* buffer, int32_t width, int32_t height, GLint internalFormat = GL_RGBA);
	Texture(const Texture& other) = delete;
	Texture(Texture&& other);
	Texture& operator=(const Texture& rhs) = delete;
	Texture& operator=(Texture&& rhs);
	~Texture();
	int32_t getWidth() const { return width; }
	int32_t getHeight() const { return height; }
	uint64_t getSize() const { return (width * height * sizeof(int32_t) + sizeof(Texture)); }
	explicit operator bool() const;
	void bind(int32_t slot = 0) const;
	GLuint getTextureId() const { return texture; }
};