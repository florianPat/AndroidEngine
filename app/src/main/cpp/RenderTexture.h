#pragma once

#ifdef VK_GRAPHICS
#include "VulkanRenderTexture.h"
#define RenderTexture VulkanRenderTexture
#endif

#ifdef GL_GRAPHICS
#include "OGLRenderTexture.h"
#define RenderTexture OGLRenderTexture
#endif