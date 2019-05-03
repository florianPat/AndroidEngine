#include "RenderWindow.h"
#include "TouchInput.h"
#include "Utils.h"
#include "Types.h"
#include "Utils.h"
#include "Ifstream.h"
#include "GLUtils.h"
#include "Font.h"

void RenderWindow::AppEventCallback(android_app * app, int32_t command)
{
    RenderWindow& renderWindow = *(RenderWindow*)app->userData;
    renderWindow.processAppEvent(command);
}

int RenderWindow::processInputEvent(AInputEvent * event)
{
    if (!initFinished)
        return 0;

    int eventType = AInputEvent_getType(event);

    switch (eventType)
    {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            int source = AInputEvent_getSource(event);
            switch (source)
            {
                case AINPUT_SOURCE_TOUCHSCREEN:
                {
                    int action = AMotionEvent_getAction(event);
                    if (action == AMOTION_EVENT_ACTION_MOVE || action == AMOTION_EVENT_ACTION_DOWN)
                    {
                        getAndSetTouchInputPos(event);
                        TouchInput::setTouched(true);
                    }
                    else if (action == AMOTION_EVENT_ACTION_UP)
                    {
                        getAndSetTouchInputPos(event);
                        TouchInput::setTouched(true);
                        TouchInput::setShouldUp(true);
                    }
                    else if (action == AMOTION_EVENT_ACTION_CANCEL)
                    {
                        TouchInput::setTouched(false);
                    }
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

int RenderWindow::InputEventCallback(android_app * app, AInputEvent * event)
{
    RenderWindow& renderWindow = *(RenderWindow*)app->userData;
    return renderWindow.processInputEvent(event);
}

RenderWindow::RenderWindow(android_app * app, int width, int height, ViewportType viewportType) : app(app), clock(),
                                                                                                  renderWidth(width), renderHeight(height),
                                                                                                  assetManager(app->activity->assetManager),
                                                                                                  view(renderWidth, renderHeight), orhtoProj(view.getOrthoProj()),
                                                                                                  viewportType(viewportType)
{
    app->userData = this;
    app->onAppCmd = AppEventCallback;
    app->onInputEvent = InputEventCallback;

    processEvents();
}

bool RenderWindow::processEvents()
{
    if (TouchInput::getShouldUp())
    {
        TouchInput::setShouldUp(false);
        TouchInput::setTouched(false);
    }

    int32_t event;
    android_poll_source* source;

    while ((ALooper_pollAll(initFinished ? 0 : -1, 0, &event, (void**)&source)) >= 0)
    {
        assert(source != nullptr);
        source->process(app, source);

        if (app->destroyRequested || (!running))
        {
            return false;
        }
    }

    return true;
}

void RenderWindow::clear()
{
    CallGL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    CallGL(glClear(GL_COLOR_BUFFER_BIT));
}

void RenderWindow::close()
{
    ANativeActivity_finish(app->activity);
    running = false;
}

void RenderWindow::draw(const Sprite & sprite)
{
    const Texture* texture = sprite.getTexture();
    assert(*texture);

    texture->bind();

    float texRectLeft = ((float)sprite.getTextureRect().left) / texture->getWidth();
    float texRectTop = ((float)sprite.getTextureRect().top) / texture->getHeight();
    float texRectRight = ((float)sprite.getTextureRect().getRight()) / texture->getWidth();
    float texRectBottom = ((float)sprite.getTextureRect().getBottom()) / texture->getHeight();

    Vector2f texCoord[4] = { { texRectLeft, texRectTop },
                             { texRectRight, texRectTop },
                             { texRectRight, texRectBottom },
                             { texRectLeft, texRectBottom } };

    float vertices[] = {0.0f, 0.0f, texCoord[0].x, texCoord[0].y,
                        1.0f, 0.0f, texCoord[1].x, texCoord[1].y,
                        1.0f, 1.0f, texCoord[2].x, texCoord[2].y,
                        0.0f, 1.0f, texCoord[3].x, texCoord[3].y };

    unsigned int indices[] = { 0, 2, 3, 0, 1, 2 };

    VertexBuffer vb = VertexBuffer(vertices, sizeof(vertices));
    IndexBuffer ib = IndexBuffer(indices, arrayCount(indices));

    vb.bind();

    VertexLayouts va;
    va.addAttribute(2, GL_FLOAT);
    va.addAttribute(2, GL_FLOAT);
    va.set();

    ib.bind();

    Mat4x4 mvp = orhtoProj * sprite.getTransform();

    Color c = sprite.getColor();

    shaderSprite.bind();
    shaderSprite.setUniformMat4f("u_mvp", mvp);
    shaderSprite.setUniform4f("u_color", c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);

    CallGL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, 0));
}

void RenderWindow::draw(const RectangleShape & rect)
{
    float vertices[] = { 0.0f, 0.0f,
                         1.0f, 0.0f,
                         1.0f, 1.0f,
                         0.0f, 1.0f };

    unsigned int indices[] = { 0, 2, 3, 0, 1, 2 };

    VertexBuffer vb = VertexBuffer(vertices, sizeof(vertices));
    IndexBuffer ib = IndexBuffer(indices, arrayCount(indices));

    vb.bind();

    VertexLayouts va;
    va.addAttribute(2, GL_FLOAT);
    va.set();

    ib.bind();

    Mat4x4 mvp = orhtoProj * rect.getTransform();

    Color c = rect.getFillColor();

    shaderRectShape.bind();
    shaderRectShape.setUniformMat4f("u_mvp", mvp);
    shaderRectShape.setUniform4f("u_color", c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);

    CallGL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, 0));
}

//TODO: FIX!
void RenderWindow::draw(const CircleShape & circle)
{
    //Code by: https://gist.github.com/linusthe3rd/803118
    static constexpr int triangleAmount = 20;

    float twicePi = 2.0f * PIf;

    float radius = circle.getRadius();

    float vertices[triangleAmount * 2];
    for (int i = 0, vertCounter = 0; i < triangleAmount; i++)
    {
        vertices[vertCounter++] = radius * cosf(i * twicePi / triangleAmount);
        vertices[vertCounter++] = radius * sinf(i * twicePi / triangleAmount);
    }

    VertexBuffer vb = VertexBuffer(vertices, sizeof(vertices));
    vb.bind();
    VertexLayouts va;
    va.addAttribute(2, GL_FLOAT);
    va.set();

    Mat4x4 mvp = orhtoProj * circle.getTransform();

    Color c = circle.getFillColor();

    shaderRectShape.bind();
    shaderRectShape.setUniformMat4f("u_mvp", mvp);
    shaderRectShape.setUniform4f("u_color", c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);

    CallGL(glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(vertices)));
}

void RenderWindow::render()
{
    EGLBoolean result = eglSwapBuffers(display, surface);
    assert(result);

    if (view.updated())
        orhtoProj = view.getOrthoProj();
}

AssetManager * RenderWindow::getAssetManager()
{
    return &assetManager;
}

View & RenderWindow::getDefaultView()
{
    return view;
}

int RenderWindow::getViewportWidth() const
{
    return viewportWidth;
}

int RenderWindow::getViewportHeight() const
{
    return viewportHeight;
}

int RenderWindow::getRenderWidth() const
{
    return renderWidth;
}

int RenderWindow::getRenderHeight() const
{
    return renderHeight;
}

void RenderWindow::recoverFromContextLoss()
{
    assert(glContextLost);

    EGLConfig config;

    if (display == EGL_NO_DISPLAY)
    {
        if (!initDisplay(config))
            return;
    }

    if (surface == EGL_NO_SURFACE)
    {
        if (!initSurface(config))
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

    setupGfxGpu();

    glContextLost = false;
}

Clock & RenderWindow::getClock() const
{
    return (Clock&)clock;
}

void callback_sound(SLAndroidSimpleBufferQueueItf playerBuffer, void* context)
{
    utils::log("Sound finished!");
}

void RenderWindow::play(const Sound* snd)
{
    (*playerBuffer)->Clear(playerBuffer);

    //NOTE: Only call this if the buffer is stopped!
    /*assert((*player)->SetCallbackEventsMask(player, SL_PLAYEVENT_HEADATEND) == SL_RESULT_SUCCESS);
    assert((*playerBuffer)->RegisterCallback(playerBuffer, callback_sound, nullptr) == SL_RESULT_SUCCESS);*/

    assert((*playerBuffer)->Enqueue(playerBuffer, (void*)snd->getBuffer(), (uint) snd->getNSamples()) == SL_RESULT_SUCCESS);
}

void RenderWindow::deactivate()
{
    shaderSprite.~Shader();
    shaderRectShape.~Shader();

    stopGfx();
    stopSnd();
    stopFont();

    initFinished = false;
}

void RenderWindow::processAppEvent(int32_t command)
{
    switch (command)
    {
        case APP_CMD_WINDOW_RESIZED:
        {
            //NOTE: New viewport? and the RenderTexture also needs to query the new window size.
            break;
        }
        case APP_CMD_CONTENT_RECT_CHANGED:
        {
            //NOTE: New viewport? and the RenderTexture also needs to query the new window size.
            //"Real" client area is in content rect
            break;
        }
        case APP_CMD_INPUT_CHANGED:
        {
            utils::log("Input changed!!");
        }
        case APP_CMD_LOW_MEMORY:
        {
            utils::log("Low memory!");
            break;
        }
        case APP_CMD_INIT_WINDOW:
        {
            assert(app->window != nullptr);
            assert(!initFinished);

            if (startGfx() && startSnd() && startFont())
                setupGfxGpu();
            else
            {
                deactivate();
                close();
                break;
            }

            validNativeWindow = true;
            if (resumed && gainedFocus)
            {
                initFinished = true;
                clock.restart();
            }

            utils::log("initWindow");

            break;
        }
        case APP_CMD_DESTROY:
        {
            running = false;
            utils::log("destroy");
            break;
        }
        case APP_CMD_GAINED_FOCUS:
        {
            gainedFocus = true;
            if (resumed && validNativeWindow)
            {
                initFinished = true;
                clock.restart();
            }

            utils::log("gained Focus");

            break;
        }
        case APP_CMD_LOST_FOCUS:
        {
            gainedFocus = false;
            initFinished = false;
            utils::log("lost focus");
            break;
        }
        case APP_CMD_PAUSE:
        {
            resumed = false;
            initFinished = false;
            utils::log("pause");
            break;
        }
        case APP_CMD_RESUME:
        {
            if (validNativeWindow)
            {
                clear();
                checkIfToRecoverFromContextLoss();
                while (glContextLost)
                    recoverFromContextLoss();
            }

            resumed = true;
            if (gainedFocus && validNativeWindow)
            {
                initFinished = true;
                clock.restart();
            }

            utils::log("resume");

            break;
        }
        case APP_CMD_SAVE_STATE:
        {
            break;
        }
        case APP_CMD_START:
        {
            //first create or recreate if there is something in the save state
            utils::log("start");
            break;
        }
        case APP_CMD_STOP:
        {
            //Now the app really is not visible!
            utils::log("stop");
            break;
        }
        case APP_CMD_TERM_WINDOW:
        {
            deactivate();
            validNativeWindow = false;
            initFinished = false;
            utils::log("term window");

            break;
        }
        default:
        {
            break;
        }
    }
}

bool RenderWindow::startGfx()
{
    EGLConfig config;

    if (!initDisplay(config))
        return false;

    if (!initSurface(config))
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

    //NOTE: For ViewportType::FIT, but also needed for extend!
    float ratioScreen = (float)screenWidth / (float)screenHeight;
    float ratioGame = (float)renderWidth / (float)renderHeight;
    if (ratioScreen > ratioGame)
    {
        viewportWidth = (int)((float)screenHeight * ratioGame);
        viewportHeight = screenHeight;
    }
    else
    {
        viewportWidth = screenWidth;
        viewportHeight = (int)((float)screenWidth / ratioGame);
    }

    if (viewportType == ViewportType::EXTEND)
    {
        if (viewportWidth == screenWidth)
        {
            float remainingSpace = screenHeight - viewportHeight;
            renderHeight += (int) (remainingSpace * ((float)renderHeight / (float)viewportHeight));
            viewportHeight = screenHeight;
        }
        else if (viewportHeight == screenHeight)
        {
            float remainingSpace = screenWidth - viewportWidth;
            renderWidth += (int) (remainingSpace * ((float)renderWidth / (float)viewportWidth));
            viewportWidth = screenWidth;
        }
        else
            InvalidCodePath;

        view = View(renderWidth, renderHeight);
        orhtoProj = view.getOrthoProj();
    }

    CallGL(glViewport(0, 0, viewportWidth, viewportHeight));

    CallGL(glDisable(GL_DEPTH_TEST));
    CallGL(glEnable(GL_BLEND));
    CallGL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    return true;
}

bool RenderWindow::initDisplay(EGLConfig& config)
{
    EGLint format, numConfigs;
    constexpr EGLint DISPLAY_ATTRIBS[] = {
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

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

    if (!eglChooseConfig(display, DISPLAY_ATTRIBS, &config, 1, &numConfigs) || (numConfigs <= 0))
    {
        utils::logBreak("eglChooseConfig failed!");
        return false;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
    {
        utils::logBreak("eglGetConfigAttrib failed!");
        return false;
    }

    if (ANativeWindow_setBuffersGeometry(app->window, 0, 0, format) != 0)
    {
        utils::logBreak("ANativeWindow_setBufferGeometry failed!");
        return false;
    }

    return true;
}

bool RenderWindow::initSurface(EGLConfig& config)
{
    surface = eglCreateWindowSurface(display, config, app->window, nullptr);
    if (surface == EGL_NO_SURFACE)
    {
        utils::logBreak("eglCreateWindowSurface failed!");
        return false;
    }

    return true;
}

bool RenderWindow::initContext(EGLConfig& config)
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

void RenderWindow::stopGfx()
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

void RenderWindow::getAndSetTouchInputPos(AInputEvent * event)
{
    //Needs conversion because coord system is from topLeft, but game uses bottomLeft and other window dimensions
    float x = AMotionEvent_getX(event, 0);
    float y = (AMotionEvent_getY(event, 0) - viewportHeight) * -1.0f;

    x = x / viewportWidth * renderWidth;
    y = y / viewportHeight * renderHeight;

    TouchInput::setPosition(x, y);
}

bool RenderWindow::startSnd()
{
    const SLuint32 engineMixIfCount = 1;
    const SLInterfaceID engineMixIfs[] = { SL_IID_ENGINE };
    const SLboolean engineMixIfsReq[] = { SL_BOOLEAN_TRUE };

    const SLuint32 outputMixIfCount = 0;
    const SLInterfaceID outputMixIfs[] = {};
    const SLboolean outputMixIfsReq[] = {};

    if(slCreateEngine(&engineObj, 0, nullptr, engineMixIfCount, engineMixIfs, engineMixIfsReq) != SL_RESULT_SUCCESS)
    {
        utils::logBreak("slCreateEngine failed!");
        return false;
    }

    if((*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS)
    {
        utils::logBreak("Realize failed!");
        return false;
    }

    if((*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engine) != SL_RESULT_SUCCESS)
    {
        utils::logBreak("GetInterface failed!");
        return false;
    }

    if ((*engine)->CreateOutputMix(engine, &outputMix, outputMixIfCount, outputMixIfs, outputMixIfsReq) != SL_RESULT_SUCCESS)
    {
        utils::logBreak("CreateOutputMix failed!");
        return false;
    }

    if((*outputMix)->Realize(outputMix, SL_BOOLEAN_FALSE))
    {
        utils::logBreak("Realize failed!");
        return false;
    }

    SLDataLocator_AndroidSimpleBufferQueue dataLocatorIn = { 0 };
    dataLocatorIn.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    dataLocatorIn.numBuffers = 1;

    SLDataFormat_PCM dataFormat = { 0 };
    dataFormat.formatType = SL_DATAFORMAT_PCM;
    dataFormat.numChannels = 1;
    dataFormat.samplesPerSec = SL_SAMPLINGRATE_44_1;
    dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
    dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource dataSource = { 0 };
    dataSource.pLocator = &dataLocatorIn;
    dataSource.pFormat = &dataFormat;

    SLDataLocator_OutputMix dataLocatorOut = { 0 };
    dataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    dataLocatorOut.outputMix = outputMix;

    SLDataSink dataSink = { 0 };
    dataSink.pLocator = &dataLocatorOut;
    dataSink.pFormat = nullptr;

    const SLuint32 soundPlayerIIdCount = 2;
    const SLInterfaceID soundPlayerIIds[] = { SL_IID_PLAY, SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
    const SLboolean soundPlayerReqs[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

    if ((*engine)->CreateAudioPlayer(engine, &playerObj, &dataSource, &dataSink,
                                     soundPlayerIIdCount, soundPlayerIIds, soundPlayerReqs) != SL_RESULT_SUCCESS)
    {
        utils::logBreak("CreateAudioPlayer failed!");
        InvalidCodePath;
    }

    if ((*playerObj)->Realize(playerObj, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS)
    {
        utils::logBreak("Realize failed!");
        InvalidCodePath;
    }

    if ((*playerObj)->GetInterface(playerObj, SL_IID_PLAY, &player) != SL_RESULT_SUCCESS)
    {
        utils::logBreak("GetInterface failed!");
        InvalidCodePath;
    }

    if ((*playerObj)->GetInterface(playerObj, SL_IID_BUFFERQUEUE, &playerBuffer) != SL_RESULT_SUCCESS)
    {
        utils::logBreak("GetInterface failed!");
        InvalidCodePath;
    }

    assert((*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING) == SL_RESULT_SUCCESS);

    return true;
}

void RenderWindow::stopSnd()
{
    if (playerObj != nullptr)
    {
        (*playerObj)->Destroy(playerObj);
        playerObj = nullptr;
    }

    if (outputMix != nullptr)
    {
        (*outputMix)->Destroy(outputMix);
        outputMix = nullptr;
    }

    if (engineObj != nullptr)
    {
        (*engineObj)->Destroy(engineObj);
        engineObj = nullptr;
        engine = nullptr;
    }
}

//NOTE: Init graphics things here!
void RenderWindow::setupGfxGpu()
{
    new (&shaderSprite) Shader("SSprite", Vector<ShortString>{ "position", "texCoord" });
    new (&shaderRectShape) Shader("SRectShape", Vector<ShortString>{ "position" });

    assetManager.reloadAllRes();
}

void RenderWindow::checkIfToRecoverFromContextLoss()
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
}

Shader * RenderWindow::getSpriteShader()
{
    return &shaderSprite;
}

bool RenderWindow::startFont()
{
    int error = FT_Init_FreeType(&fontLibrary);
    if(error)
        utils::log("could not init freetype library");

    return (error == 0);
}

void RenderWindow::stopFont()
{
    FT_Done_FreeType(fontLibrary);
    fontLibrary = nullptr;
}

FT_Library RenderWindow::getFontLibrary()
{
    return fontLibrary;
}
