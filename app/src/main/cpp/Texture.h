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

	GLuint texture = 0;
	int width = 0;
	int height = 0;
public:
	bool loadFromFile(const String& filename);
	bool reloadFromFile(const String& filename);
public:
	Texture() = default;
	Texture(GLuint texture, int width, int height);
	Texture(const Texture& other) = delete;
	Texture(Texture&& other);
	Texture& operator=(const Texture& rhs) = delete;
	Texture& operator=(Texture&& rhs);
	~Texture();
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	long long getSize() const { return width * height * sizeof(int32_t); }
	explicit operator bool() const;
	void bind(int slot = 0) const;
};