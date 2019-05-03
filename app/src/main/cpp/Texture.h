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
	int width = 0;
	int height = 0;
public:
	bool loadFromFile(const String& filename);
	bool reloadFromFile(const String& filename);
public:
	Texture() = default;
	Texture(GLuint texture, int width, int height);
	Texture(const void* buffer, int width, int height, GLint internalFormat = GL_RGBA);
	Texture(const Texture& other) = delete;
	Texture(Texture&& other);
	Texture& operator=(const Texture& rhs) = delete;
	Texture& operator=(Texture&& rhs);
	~Texture();
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	long long getSize() const { return (width * height * sizeof(int32_t) + sizeof(Texture)); }
	explicit operator bool() const;
	void bind(int slot = 0) const;
};