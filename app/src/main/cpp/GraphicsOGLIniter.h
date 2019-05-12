#pragma once

#include "IGraphics.h"
#include <EGL/egl.h>

class GraphicsOGLIniter : public IGraphics
{
protected:
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    bool glContextLost = false;
    const EGLint* displayAttribs;
private:
    bool initDisplay(EGLConfig& config, ANativeWindow* nativeWindow);
    bool initSurface(EGLConfig& config, ANativeWindow* nativeWindow);
    bool initContext(EGLConfig& config);
public:
    GraphicsOGLIniter(int renderWidth, int renderHeight, ViewportType viewportType, const EGLint* displayAttribs);

    bool startGfx(ANativeWindow* nativeWindow);
    void stopGfx();
    bool checkIfToRecover();
    void recover(ANativeWindow* nativeWindow);

    bool isRecovered() const;
};