#pragma once

#ifdef VK_GRAPHICS
#include "GraphicsVK.h"
#define Graphics GraphicsVK
#endif

#ifdef GL_GRAPHICS
#include "GraphicsOGL2.h"
#define Graphics GraphicsOGL2
#endif