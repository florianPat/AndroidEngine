#include "Window.h"
#include "TouchInput.h"
#include "Utils.h"
#include "Types.h"
#include "Utils.h"
#include "Ifstream.h"
#include "Font.h"
#include "Globals.h"

void Window::AppEventCallback(android_app * app, int32_t command)
{
    Window& window = *(Window*)app->userData;
    window.processAppEvent(command);
}

int Window::processInputEvent(AInputEvent * event)
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
                    int combined = AMotionEvent_getAction(event);
                    uint action = ((uint)combined & AMOTION_EVENT_ACTION_MASK);
                    uint pointerIndex = (((uint)combined) & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

                    int pointerId = AMotionEvent_getPointerId(event, pointerIndex);

                    touchInput.inputs[pointerId].move = false;
                    touchInput.inputs[pointerId].down = false;
                    touchInput.inputs[pointerId].up = false;

                    getAndSetTouchInputPos(event, pointerId, pointerIndex);

                    switch(action)
                    {
                        case AMOTION_EVENT_ACTION_MOVE:
                        {
                            touchInput.inputs[pointerId].move = true;
                            break;
                        }
                        case AMOTION_EVENT_ACTION_DOWN:
                        {
                            touchInput.inputs[pointerId].down = true;
                            break;
                        }
                        case AMOTION_EVENT_ACTION_UP:
                        {
                            touchInput.inputs[pointerId].up = true;
                            break;
                        }
                        case AMOTION_EVENT_ACTION_POINTER_DOWN:
                        {
                            touchInput.inputs[pointerId].down = true;
                            utils::log("pointer down");
                            break;
                        }
                        case AMOTION_EVENT_ACTION_POINTER_UP:
                        {
                            touchInput.inputs[pointerId].up = true;
                            utils::log("pointer up");
                            break;
                        }
                    }
                }
            }
            break;
        }
    }

    return 0;
}

int Window::InputEventCallback(android_app * app, AInputEvent * event)
{
    Window& window = *(Window*)app->userData;
    return window.processInputEvent(event);
}

Window::Window(android_app * app, int width, int height, View::ViewportType viewportType) : app(app),
                                                                                                  clock(),
                                                                                                  assetManager(),
                                                                                                  gfx(width, height, viewportType)
{
    Globals::window = this;

    Ifstream::setAassetManager(app->activity->assetManager);

    app->userData = this;
    app->onAppCmd = AppEventCallback;
    app->onInputEvent = InputEventCallback;

    processEvents();
}

bool Window::processEvents()
{
    for(int i = 0; i < arrayCount(touchInput.inputs); ++i)
    {
        touchInput.inputs[i].up = false;
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

void Window::close()
{
    ANativeActivity_finish(app->activity);
    running = false;
}

AssetManager * Window::getAssetManager()
{
    return &assetManager;
}

Clock & Window::getClock()
{
    return clock;
}

void callback_sound(SLAndroidSimpleBufferQueueItf playerBuffer, void* context)
{
    utils::log("Sound finished!");
}

void Window::play(const Sound* snd)
{
    (*playerBuffer)->Clear(playerBuffer);

    //NOTE: Only call this if the buffer is stopped!
    /*assert((*player)->SetCallbackEventsMask(player, SL_PLAYEVENT_HEADATEND) == SL_RESULT_SUCCESS);
    assert((*playerBuffer)->RegisterCallback(playerBuffer, callback_sound, nullptr) == SL_RESULT_SUCCESS);*/

    assert((*playerBuffer)->Enqueue(playerBuffer, (void*)snd->getBuffer(), (uint) snd->getNSamples()) == SL_RESULT_SUCCESS);
}

void Window::deactivate()
{
    gfx.stopGfx();
    stopSnd();
    stopFont();

    initFinished = false;
}

void Window::processAppEvent(int32_t command)
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
            break;
        }
        case APP_CMD_LOW_MEMORY:
        {
            break;
        }
        case APP_CMD_INIT_WINDOW:
        {
            assert(app->window != nullptr);
            assert(!initFinished);

            if (gfx.startGfx(app->window) && startSnd() && startFont())
                gfx.setupGfxGpu();
            else
            {
                deactivate();
                close();
                break;
            }

            validNativeWindow = true;
            if (resumed && gainedFocus)
            {
                checkAndRecoverFromContextLoss();

                initFinished = true;
                clock.restart();
            }
            break;
        }
        case APP_CMD_DESTROY:
        {
            running = false;
            break;
        }
        case APP_CMD_GAINED_FOCUS:
        {
            gainedFocus = true;
            if (resumed && validNativeWindow)
            {
                checkAndRecoverFromContextLoss();

                initFinished = true;
                clock.restart();
            }
            break;
        }
        case APP_CMD_LOST_FOCUS:
        {
            gainedFocus = false;
            initFinished = false;
            break;
        }
        case APP_CMD_PAUSE:
        {
            resumed = false;
            initFinished = false;
            break;
        }
        case APP_CMD_RESUME:
        {
            resumed = true;
            if (gainedFocus && validNativeWindow)
            {
                checkAndRecoverFromContextLoss();

                initFinished = true;
                clock.restart();
            }
            break;
        }
        case APP_CMD_SAVE_STATE:
        {
            break;
        }
        case APP_CMD_START:
        {
            //first create or recreate if there is something in the save state
            recreating = (bool) app->stateSaved;
            break;
        }
        case APP_CMD_STOP:
        {
            //Now the app really is not visible!
            break;
        }
        case APP_CMD_TERM_WINDOW:
        {
            deactivate();
            validNativeWindow = false;
            initFinished = false;
            break;
        }
        default:
        {
            break;
        }
    }
}

void Window::getAndSetTouchInputPos(AInputEvent* event, int pointerId, uint pointerIndex)
{
    //Needs conversion because coord system is from topLeft, but game uses bottomLeft and other window dimensions
    float x = AMotionEvent_getX(event, pointerIndex);
    float y = gfx.screenHeight - AMotionEvent_getY(event, pointerIndex);

    const Vector2i& viewportSize = gfx.getDefaultView().getSize();

    x = x / gfx.screenWidth * viewportSize.x;
    y = y / gfx.screenHeight * viewportSize.y;

    touchInput.inputs[pointerId].touchPos = { x, y };
}

bool Window::startSnd()
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

void Window::stopSnd()
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

bool Window::startFont()
{
    int error = FT_Init_FreeType(&fontLibrary);
    if(error)
        utils::log("could not init freetype library");

    return (error == 0);
}

void Window::stopFont()
{
    FT_Done_FreeType(fontLibrary);
    fontLibrary = nullptr;
}

FT_Library Window::getFontLibrary()
{
    return fontLibrary;
}

Graphics& Window::getGfx()
{
    return gfx;
}

void Window::checkAndRecoverFromContextLoss()
{
    gfx.clear();

    if(gfx.checkIfToRecover())
    {
        while(!gfx.isRecovered())
            gfx.recover(app->window);

        gfx.setupGfxGpu();
        assetManager.reloadAllRes();
    }
    else if(recreating)
    {
        gfx.setupGfxGpu();
        assetManager.reloadAllRes();
    }
}

const TouchInput& Window::getTouchInput() const
{
    return touchInput;
}
