#include "PNGReadCallback.h"
#include "Ifstream.h"
#include "Types.h"

void callbackReadTransform(png_structp ptr, png_row_infop rowInfo, png_bytep data)
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

void callbackReadPng(png_structp pngStruct, png_bytep data, png_size_t size)
{
    Ifstream* asset = ((Ifstream*)png_get_io_ptr(pngStruct));
    asset->read(data, (uint32_t)size);
}
