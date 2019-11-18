#pragma once

#ifdef VK_GRAPHICS
#include "VulkanTexture.h"
#define Texture VulkanTexture
#endif

#ifdef GL_GRAPHICS
#include "OGLTexture.h"
#define Texture OGLTexture
#endif