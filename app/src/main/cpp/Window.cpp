#include "Window.h"
#include "TouchInput.h"
#include "Utils.h"
#include "Types.h"
#include "Utils.h"
#include "Ifstream.h"
#include "Font.h"
#include "Globals.h"
#include "JNIUtils.h"
#include "EventDestroyApp.h"
#include "EventStopApp.h"
#include "EventResumeApp.h"
#include "EventManager.h"
#include "DirWalker.h"

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

void Window::deactivate()
{
    gfx.stopGfx();
    audio.stopSnd();
    stopFont();
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

            if (gfx.startGfx(app->window) && audio.startSnd() && startFont())
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
                finishInit();
            }
            break;
        }
        //NOTE: This event is NOT guaranteed to be called (if app gets wiped out of memory by system and is in background, this
        // will not trigger)
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
                finishInit();
            }
            break;
        }
        case APP_CMD_LOST_FOCUS:
        {
            if(initFinished)
            {
                finishDeinit();
            }

            gainedFocus = false;
            break;
        }
        case APP_CMD_PAUSE:
        {
            if(initFinished)
            {
                finishDeinit();
            }

            resumed = false;
            break;
        }
        case APP_CMD_RESUME:
        {
            resumed = true;

            if (gainedFocus && validNativeWindow)
            {
                finishInit();
            }
            break;
        }
        case APP_CMD_SAVE_STATE:
        {
            break;
        }
        case APP_CMD_START:
        {
            assert(app->activity != nullptr);
            assert(app->activity->env != nullptr);
            assert(app->activity->clazz != nullptr);
            assert(app->activity->assetManager != nullptr);
            jniUtils::activity = app->activity->clazz;
            Ifstream::setAassetManager(app->activity->assetManager);
            DirWalker::setAassetManager(app->activity->assetManager);
            //TODO: Use app->activity->internalDataPath for saving??

            jniUtils::vm = app->activity->vm;
            jniUtils::vm->AttachCurrentThreadAsDaemon(&jniUtils::jniEnv, nullptr);

            jclass activityClass = jniUtils::jniEnv->FindClass("android/app/NativeActivity");
            assert(activityClass);
            jmethodID getClassLoader = jniUtils::jniEnv->GetMethodID(activityClass,"getClassLoader", "()Ljava/lang/ClassLoader;");
            assert(getClassLoader);
            jobject classLoaderObject = jniUtils::jniEnv->CallObjectMethod(jniUtils::activity, getClassLoader);
            assert(classLoaderObject);
            jniUtils::classLoader.object = jniUtils::jniEnv->NewGlobalRef(classLoaderObject);
            jclass classLoaderClass = jniUtils::jniEnv->FindClass("java/lang/ClassLoader");
            assert(classLoaderClass);
            jmethodID findClassMethod = jniUtils::jniEnv->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
            jniUtils::classLoader.findClassMethod = findClassMethod;
            assert(jniUtils::classLoader.findClassMethod);

            nativeThreadQueue.startThreads();

            //first create or recreate if there is something in the save state
            recreating = (bool) app->stateSaved;
            break;
        }
        case APP_CMD_STOP:
        {
            //Now the app really is not visible!
            assert(Globals::eventManager != nullptr);
            Globals::eventManager->TriggerEvent<EventStopApp>();
            nativeThreadQueue.flushFrom(threadQueueEventBeginning);

            nativeThreadQueue.endThreads();

            jniUtils::jniEnv->DeleteGlobalRef(jniUtils::classLoader.object);
            jniUtils::vm->DetachCurrentThread();

            break;
        }
        case APP_CMD_TERM_WINDOW:
        {
            if(initFinished)
            {
                finishDeinit();
            }

            deactivate();
            validNativeWindow = false;
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

NativeThreadQueue& Window::getNativeThreadQueue()
{
    return nativeThreadQueue;
}

Audio& Window::getAudio()
{
    return audio;
}

void Window::finishInit()
{
    checkAndRecoverFromContextLoss();

    if(Globals::eventManager != nullptr)
    {
        Globals::eventManager->TriggerEvent<EventResumeApp>();
        nativeThreadQueue.flushFrom(threadQueueEventBeginning);
    }

    audio.startStream();

    initFinished = true;
    clock.restart();
}

void Window::finishDeinit()
{
    audio.stopStream();

    initFinished = false;
}

void Window::callToGetEventJobsBeginning()
{
    threadQueueEventBeginning = nativeThreadQueue.getSize();
}
