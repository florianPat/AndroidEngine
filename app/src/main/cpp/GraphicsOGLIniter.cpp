#include "GraphicsOGLIniter.h"

#include "GLUtils.h"
#include <GLES2/gl2.h>
#include "android_native_app_glue.h"

void GraphicsOGLIniter::recover(ANativeWindow* nativeWindow)
{
    assert(glContextLost);

    EGLConfig config;

    if (display == EGL_NO_DISPLAY)
    {
        if (!initDisplay(config, nativeWindow))
            return;
    }

    if (surface == EGL_NO_SURFACE)
    {
        if (!initSurface(config, nativeWindow))
            return;
    }

    if (context == EGL_NO_CONTEXT)
    {
        if (!initContext(config))
            return;
    }

    if (!eglMakeCurrent(display, surface, surface, context) ||
        !eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth) || !eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight) ||
        (screenWidth <= 0) || (screenHeight <= 0))
    {
        utils::logBreak("eglMakeCurrent failed!");
        return;
    }

    glContextLost = false;
}

bool GraphicsOGLIniter::isRecovered() const
{
    return (!glContextLost);
}

bool GraphicsOGLIniter::initSurface(EGLConfig& config, ANativeWindow* nativeWindow)
{
    surface = eglCreateWindowSurface(display, config, nativeWindow, nullptr);
    if (surface == EGL_NO_SURFACE)
    {
        utils::logBreak("eglCreateWindowSurface failed!");
        return false;
    }

    return true;
}

bool GraphicsOGLIniter::initContext(EGLConfig& config)
{
    constexpr EGLint CONTEXT_ATTRIBS[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };

    context = eglCreateContext(display, config, 0, CONTEXT_ATTRIBS);
    if (context == EGL_NO_CONTEXT)
    {
        utils::logBreak("eglCreateContext failed!");
        return false;
    }

    return true;
}

void GraphicsOGLIniter::stopGfx()
{
    if (display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(display, context);
            context = EGL_NO_CONTEXT;
        }

        if (surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(display, surface);
            surface = EGL_NO_SURFACE;
        }

        eglTerminate(display);
        display = EGL_NO_DISPLAY;
    }
}

bool GraphicsOGLIniter::checkIfToRecover()
{
    EGLBoolean result;
    result = eglSwapBuffers(display, surface);
    if (result == GL_FALSE)
    {
        while (glGetError() != GL_NO_ERROR);

        while (GLenum errorCode = glGetError())
        {
            switch (errorCode)
            {
                case EGL_NOT_INITIALIZED:
                case EGL_BAD_DISPLAY:
                {
                    if (display != EGL_NO_DISPLAY)
                    {
                        eglTerminate(display);
                        display = EGL_NO_DISPLAY;
                    }

                    glContextLost = true;
                    break;
                }
                case EGL_CONTEXT_LOST:
                {
                    if (context != EGL_NO_CONTEXT)
                    {
                        eglDestroyContext(display, context);
                        context = EGL_NO_CONTEXT;
                    }

                    glContextLost = true;
                    break;
                }
                case EGL_BAD_SURFACE:
                {
                    if (surface != EGL_NO_SURFACE)
                    {
                        eglDestroySurface(display, surface);
                        surface = EGL_NO_SURFACE;
                    }

                    glContextLost = true;
                    break;
                }
                default:
                {
                    utils::logFBreak("OpenGL error: [%d] occured in function: %s, line: %d, file: %s '\n'", errorCode, "render", __LINE__, __FILE__);
                    break;
                }
            }
        }
    }

    return glContextLost;
}

bool GraphicsOGLIniter::initDisplay(EGLConfig& config, ANativeWindow* nativeWindow)
{
    EGLint format, numConfigs;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
    {
        utils::logBreak("eglGetDisplay failed!");
        return false;
    }

    if (!eglInitialize(display, 0, 0))
    {
        utils::logBreak("egInitialize failed!");
        return false;
    }

    if (!eglChooseConfig(display, displayAttribs, &config, 1, &numConfigs) || (numConfigs <= 0))
    {
        utils::logBreak("eglChooseConfig failed!");
        return false;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
    {
        utils::logBreak("eglGetConfigAttrib failed!");
        return false;
    }

    if (ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format) != 0)
    {
        utils::logBreak("ANativeWindow_setBufferGeometry failed!");
        return false;
    }

    return true;
}

bool GraphicsOGLIniter::startGfx(ANativeWindow* nativeWindow)
{
    EGLConfig config;

    if (!initDisplay(config, nativeWindow))
        return false;

    if (!initSurface(config, nativeWindow))
        return false;

    if (!initContext(config))
        return false;

    if (!eglMakeCurrent(display, surface, surface, context) ||
        !eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth) || !eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight) ||
        (screenWidth <= 0) || (screenHeight <= 0))
    {
        utils::logBreak("eglMakeCurrent failed!");
        return false;
    }

    if (eglSwapInterval(display, 1) == EGL_FALSE)
    {
        utils::logBreak("eglSwapInteral failed!");
        return false;
    }

    view = View(renderWidth, renderHeight, screenWidth, screenHeight, viewportType);

    CallGL(glViewport(0, 0, view.getViewportSize().x, view.getViewportSize().y));

    CallGL(glDisable(GL_DEPTH_TEST));
    CallGL(glEnable(GL_BLEND));
    CallGL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    return true;
}

GraphicsOGLIniter::GraphicsOGLIniter(int renderWidth, int renderHeight,
                                     View::ViewportType viewportType,
                                     const EGLint* displayAttribs) : IGraphics(renderWidth, renderHeight),
                                                                     displayAttribs(displayAttribs),
                                                                     viewportType(viewportType),
                                                                     view()
{
}
