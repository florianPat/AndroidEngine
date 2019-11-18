#pragma once

#include <png.h>

void callbackReadPng(png_structp pngStruct, png_bytep data, png_size_t size);
void callbackReadTransform(png_structp ptr, png_row_infop rowInfo, png_bytep data);