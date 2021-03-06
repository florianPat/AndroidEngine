#pragma once

#include "android_native_app_glue.h"
#include <GLES2/gl2.h>
#include "Sprite.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexLayout.h"
#include "Shader.h"
#include "Mat4x4.h"
#include "RectangleShape.h"
#include "CircleShape.h"
#include "UniquePtr.h"
#include "GraphicsOGLIniter.h"

class GraphicsOGL2 : public GraphicsOGLIniter
{
    struct Vertex
    {
        Vector2f position;
        Vector2f tex;
        float colorR, colorG, colorB, colorA;
        float mvMatrix[6];
    };
private:
    Shader shaderSprite;
    Shader shaderRectShape;
    Mat4x4 orhtoProj;
    int32_t nTextureUnits = 0;
    IndexBuffer ib;
    VertexBuffer vb;
    GLuint currentBoundTexture = (GLuint)-1;
    static constexpr uint32_t NUM_SPRITES_TO_BATCH = 32;
    static constexpr uint32_t NUM_VERTICES_TO_BATCH = NUM_SPRITES_TO_BATCH * 4;
    uint32_t nSpritesBatched = 0;
    UniquePtr<Vertex[]> vertices;
    bool isFastRectDrawing = false;

    static constexpr EGLint DISPLAY_ATTRIBS[] = {
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };
private:
    void setupSpriteRendering();
    uint32_t nVerticesBatched() const;
public:
    GraphicsOGL2(uint32_t renderWidth, uint32_t renderHeight, View::ViewportType viewportType);

    void setupGfxGpu();
    void freeGfxGpu();

    void clear();
    void bindOtherOrthoProj(const Mat4x4& otherOrthoProj);
    void unbindOtherOrthoProj();
    void draw(const Sprite& sprite);
    void draw(const RectangleShape& rect);
    void draw(const CircleShape& circle);
    void flush();
    void render();

    void startFastRectDrawing();
    void stopFastRectDrawing();
};