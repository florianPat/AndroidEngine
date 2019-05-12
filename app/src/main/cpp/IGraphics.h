#pragma once

#include "View.h"

struct IGraphics
{
    enum class ViewportType
    {
        FIT,
        EXTEND
    };
protected:
    int renderWidth = 0, renderHeight = 0;
    int screenWidth = 0, screenHeight = 0;
    int viewportWidth = 0, viewportHeight = 0;
    ViewportType viewportType;
public:
    IGraphics(int renderWidth, int renderHeight, ViewportType viewportType)
        : renderWidth(renderWidth), renderHeight(renderHeight), viewportType(viewportType)
    {}
    IGraphics(const IGraphics& other) = delete;
    IGraphics& operator=(const IGraphics& rhs) = delete;
    IGraphics(IGraphics&& other) = default;
    IGraphics& operator=(IGraphics&& rhs) = default;

    int getViewportWidth() const
    {
        return viewportWidth;
    }
    int getViewportHeight() const
    {
        return viewportHeight;
    }
    int getRenderWidth() const
    {
        return renderWidth;
    }
    int getRenderHeight() const
    {
        return renderHeight;
    }

    //NOTE: This is a blueprint to implement.
    // But not through virtual function calls, just by deriving and making the methods
    /*
    bool startGfx();
    void stopGfx();
    void checkIfToRecover();
    void recover();

    void setupGfxGpu();

    void clear();
    void bindOtherOrthoProj(const Mat4x4& otherOrthoProj);
    void unbindOtherOrthoProj();
    void draw(const Sprite& sprite);
    void draw(const RectangleShape& rect);
    void draw(const CircleShape& circle);
    void flush();
    void render();
     */
};