#include "Texture.h"
//NOTE: In pngrutil.c I made a note!
#include <png.h>
#include "Utils.h"
#include "Ifstream.h"
#include "GLUtils.h"
#include "Types.h"

static void callbackReadPng(png_structp pngStruct, png_bytep data, png_size_t size) {
	Ifstream* asset = ((Ifstream*)png_get_io_ptr(pngStruct));
	asset->read(data, (uint32_t)size);
}

static void callbackReadTransform(png_structp ptr, png_row_infop rowInfo, png_bytep data)
{
    assert(rowInfo->bit_depth == 8); // 1 byte, uchar
    assert(rowInfo->rowbytes == (rowInfo->width * sizeof(uint32_t)));
    assert(rowInfo->color_type == PNG_COLOR_TYPE_RGB || rowInfo->color_type == PNG_COLOR_TYPE_RGBA);

    if(rowInfo->color_type == PNG_COLOR_TYPE_RGBA)
    {
        assert(rowInfo->channels == 4); //RGBA
        assert(rowInfo->pixel_depth == (4 * 8));

        for(int32_t i = 0; i < rowInfo->rowbytes; i += 4)
        {
            float alpha = data[i + 3] / 255.0f;

            data[i + 0] = (uint8_t) (data[i + 0] / 255.0f * alpha * 255.0f);
            data[i + 1] = (uint8_t) (data[i + 1] / 255.0f * alpha * 255.0f);
            data[i + 2] = (uint8_t) (data[i + 2] / 255.0f * alpha * 255.0f);
        }
    }
}

void Texture::reloadFromFile(const String& filename)
{
	if (texture != 0)
	{
		CallGL(glDeleteTextures(1, &texture));
	}

	new (this) Texture(filename);
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

void Texture::bind(int32_t slot) const
{
	CallGL(glActiveTexture(GL_TEXTURE0 + slot));
	CallGL(glBindTexture(GL_TEXTURE_2D, texture));
}

Texture::Texture(const void* buffer, int32_t width, int32_t height, GLint internalFormat)
{
	CallGL(glGenTextures(1, &texture));
	CallGL(glActiveTexture(GL_TEXTURE0));
	CallGL(glBindTexture(GL_TEXTURE_2D, texture));

	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pixeld ? GL_NEAREST : GL_LINEAR));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixeld ? GL_NEAREST : GL_LINEAR));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	CallGL(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, buffer));

	this->width = width;
	this->height = height;
}

Texture::Texture(const String& filename)
{
	Ifstream asset;
	asset.open(filename);

	if (!asset)
		InvalidCodePath;

	png_byte header[8];
	png_structp png = nullptr;
	png_infop info = nullptr;
	bool transparency;

	asset.read(header, sizeof(header));
	if (png_sig_cmp(header, 0, 8) != 0)
		InvalidCodePath;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png)
		InvalidCodePath;
	info = png_create_info_struct(png);
	if (!info)
	{
		png_destroy_read_struct(&png, nullptr, nullptr);
		InvalidCodePath;
	}

	png_set_read_fn(png, &asset, callbackReadPng);
	png_set_read_user_transform_fn(png, callbackReadTransform);

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

	GLint format;
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
			png_destroy_read_struct(&png, &info, nullptr);
			InvalidCodePath;
			break;
		}
	}
	png_read_update_info(png, info);

	png_size_t rowSize = png_get_rowbytes(png, info);
	if (rowSize <= 0)
	{
		png_destroy_read_struct(&png, &info, nullptr);
		InvalidCodePath;
	}

	png_byte* image = new png_byte[rowSize * height];
	png_bytep* rowPtrs = new png_bytep[height];

	for (uint32_t i = 0, otherI = height - 1; i < height; ++i, --otherI)
	{
		rowPtrs[otherI] = &image[i * rowSize];
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
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	CallGL(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image));

	delete[] image;

	this->width = width;
	this->height = height;
}
