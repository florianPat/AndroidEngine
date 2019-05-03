#include "Texture.h"
//NOTE: In pngrutil.c I made a note!
#include <png.h>
#include "Utils.h"
#include "Ifstream.h"
#include "GLUtils.h"

void callback_readPng(png_structp pStruct, png_bytep pData, png_size_t pSize) {
	Ifstream* asset = ((Ifstream*)png_get_io_ptr(pStruct));
	asset->read(pData, pSize);
}

bool Texture::loadFromFile(const String & filename)
{
	Ifstream asset;
	asset.open(filename);

	if (!asset)
		return false;

	GLint format;
	png_byte header[8];
	png_structp png = nullptr;
	png_infop info = nullptr;
	png_byte* image = nullptr;
	png_bytep* rowPtrs = nullptr;
	png_int_32 rowSize;
	bool transparency;

	asset.read(header, sizeof(header));
	if (png_sig_cmp(header, 0, 8) != 0)
		return false;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png)
		return false;
	info = png_create_info_struct(png);
	if (!info)
		return false;

	png_set_read_fn(png, &asset, callback_readPng);

	png_set_sig_bytes(png, 8);
	png_read_info(png, info);
	png_int_32 depth, colorType;
	png_uint_32 width, height;
	png_get_IHDR(png, info, &width, &height, &depth, &colorType, nullptr, nullptr, nullptr);

	transparency = false;
	if (png_get_valid(png, info, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png);
		transparency = true;
	}

	if (depth < 8)
	{
		png_set_packing(png);
	}
	else if (depth == 16)
	{
		png_set_strip_16(png);
	}

	switch (colorType)
	{
		case PNG_COLOR_TYPE_PALETTE:
		{
			png_set_palette_to_rgb(png);
			format = transparency ? GL_RGBA : GL_RGB;
			break;
		}
		case PNG_COLOR_TYPE_RGB:
		{
			format = transparency ? GL_RGBA : GL_RGB;
			break;
		}
		case PNG_COLOR_TYPE_RGBA:
		{
			format = GL_RGBA;
			break;
		}
		case PNG_COLOR_TYPE_GRAY:
		{
			png_set_expand_gray_1_2_4_to_8(png);
			format = transparency ? GL_LUMINANCE_ALPHA : GL_LUMINANCE;
			break;
		}
		case PNG_COLOR_TYPE_GA:
		{
			png_set_expand_gray_1_2_4_to_8(png);
			format = GL_LUMINANCE_ALPHA;
			break;
		}
		default:
		{
			InvalidCodePath;
			break;
		}
	}
	png_read_update_info(png, info);

	rowSize = png_get_rowbytes(png, info);
	if (rowSize <= 0)
		return false;

	image = new png_byte[rowSize * height];
	rowPtrs = new png_bytep[height];

	for (uint32_t i = 0; i < height; ++i)
	{
		rowPtrs[height - (i + 1)] = image + i * rowSize;
	}
	png_read_image(png, rowPtrs);

	png_destroy_read_struct(&png, &info, nullptr);
	delete[] rowPtrs;
	asset.close();

	CallGL(glGenTextures(1, &texture));
	CallGL(glActiveTexture(GL_TEXTURE0));
	CallGL(glBindTexture(GL_TEXTURE_2D, texture));

	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pixeld ? GL_NEAREST : GL_LINEAR));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixeld ? GL_NEAREST : GL_LINEAR));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	CallGL(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image));

	delete[] image;

	this->width = width;
	this->height = height;

	return true;
}

bool Texture::reloadFromFile(const String& filename)
{
	if (texture != 0)
	{
		CallGL(glDeleteTextures(1, &texture));
	}

	return loadFromFile(filename);
}

Texture::Texture(GLuint texture, int width, int height) : texture(texture), width(width), height(height)
{
}

Texture::Texture(Texture && other) : texture(std::exchange(other.texture, 0)), width(std::exchange(other.width, 0)),
									 height(std::exchange(other.height, 0))
{
}

Texture & Texture::operator=(Texture && rhs)
{
	this->~Texture();

	texture = std::exchange(rhs.texture, 0);
	width = std::exchange(rhs.width, 0);
	height = std::exchange(rhs.height, 0);

	return *this;
}

Texture::~Texture()
{
    CallGL(glDeleteTextures(1, &texture));
    width = 0;
}

Texture::operator bool() const
{
	return (width != 0);
}

void Texture::bind(int slot) const
{
	CallGL(glActiveTexture(GL_TEXTURE0 + slot));
	CallGL(glBindTexture(GL_TEXTURE_2D, texture));
}

Texture::Texture(const void* buffer, int width, int height, GLint internalFormat)
{
	CallGL(glGenTextures(1, &texture));
	CallGL(glActiveTexture(GL_TEXTURE0));
	CallGL(glBindTexture(GL_TEXTURE_2D, texture));

	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pixeld ? GL_NEAREST : GL_LINEAR));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixeld ? GL_NEAREST : GL_LINEAR));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	CallGL(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, buffer));

	this->width = width;
	this->height = height;
}
