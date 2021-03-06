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

int32_t Window::processInputEvent(AInputEvent * event)
{
    if (!initFinished)
        return 0;

    int32_t eventType = AInputEvent_getType(event);
    switch (eventType)
    {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            int32_t source = AInputEvent_getSource(event);
            switch (source)
            {
                case AINPUT_SOURCE_TOUCHSCREEN:
                {
                    int32_t combined = AMotionEvent_getAction(event);
                    uint32_t action = ((uint32_t)combined & AMOTION_EVENT_ACTION_MASK);
                    uint32_t pointerIndex = (((uint32_t)combined) & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

                    int32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);

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
                        case AMOTION_EVENT_ACTION_POINTER_DOWN:
                        {
                            touchInput.inputs[pointerId].down = true;
                            break;
                        }
                        case AMOTION_EVENT_ACTION_UP:
                        case AMOTION_EVENT_ACTION_POINTER_UP:
                        {
                            touchInput.inputs[pointerId].up = true;
                            break;
                        }
                    }
                }
            }
            break;
        }
        case AINPUT_EVENT_TYPE_KEY:
        {
            int32_t action = AKeyEvent_getAction(event);
            if(action != AKEY_EVENT_ACTION_UP)
            {
                bool capitalize = false;
                int32_t metaState = AKeyEvent_getMetaState(event);
                if((metaState & AMETA_CAPS_LOCK_ON) || (metaState & AMETA_SHIFT_ON))
                {
                    capitalize = true;
                }

                uint32_t repeatCount = (uint32_t)AKeyEvent_getRepeatCount(event);

                int32_t keyCode = AKeyEvent_getKeyCode(event);
                if(keyCode >= AKEYCODE_A && keyCode <= AKEYCODE_Z)
                {
                    if(capitalize)
                        keyboardInput.text.append(repeatCount + 1, (char) (keyCode - AKEYCODE_A + 'A'));
                    else
                        keyboardInput.text.append(repeatCount + 1, (char) (keyCode - AKEYCODE_A + 'a'));
                }
                else if(keyCode >= AKEYCODE_0 && keyCode <= AKEYCODE_9)
                    keyboardInput.text.append(repeatCount + 1, (char) (keyCode - AKEYCODE_0 + '0'));
                else if(keyCode == AKEYCODE_CLEAR)
                    keyboardInput.text.clear();
                else if(keyCode == AKEYCODE_COMMA)
                    keyboardInput.text.push_back(',');
                else if(keyCode == AKEYCODE_PERIOD)
                    keyboardInput.text.push_back('.');
                else if(keyCode == AKEYCODE_SPACE)
                    keyboardInput.text.push_back(' ');
                else if(keyCode == AKEYCODE_DEL && (!keyboardInput.text.empty()))
                    keyboardInput.text.pop_back();
                //TODO: Add more keys!!
            }
            break;
        }
    }

    return 0;
}

int32_t Window::InputEventCallback(android_app * app, AInputEvent * event)
{
    Window& window = *(Window*)app->userData;
    return window.processInputEvent(event);
}

Window::Window(android_app * app, int32_t width, int32_t height, View::ViewportType viewportType) : app(app),
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
    for(int32_t i = 0; i < arrayCount(touchInput.inputs); ++i)
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
    gfx.freeGfxGpu();
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

void Window::getAndSetTouchInputPos(AInputEvent* event, int32_t pointerId, uint32_t pointerIndex)
{
    //Needs conversion because coord system is from topLeft, but game uses bottomLeft and other window dimensions
    float x = AMotionEvent_getX(event, pointerIndex);
    float y = gfx.screenHeight - AMotionEvent_getY(event, pointerIndex);

    const Vector2ui& viewportSize = gfx.getDefaultView().getSize();

    x = x / gfx.screenWidth * viewportSize.x;
    y = y / gfx.screenHeight * viewportSize.y;

    touchInput.inputs[pointerId].touchPos = { x, y };
}

bool Window::startFont()
{
    int32_t error = FT_Init_FreeType(&fontLibrary);
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

void Window::showKeyboard()
{
    // NOTE: ANativeActivity_showSoftInput()(app->activity, 0); does not work. @see https://stackoverflow.com/questions/5864790/how-to-show-the-soft-keyboard-on-native-activity
    auto [classInputMethodManager, inputMethodManager, decorView] = getKeyboardJni();

    // Runs lInputMethodManager.showSoftInput(...).
    jmethodID methodShowSoftInput = jniUtils::jniEnv->GetMethodID(classInputMethodManager, "showSoftInput","(Landroid/view/View;I)Z");
    assert(methodShowSoftInput);
    jboolean result = jniUtils::jniEnv->CallBooleanMethod(inputMethodManager, methodShowSoftInput, decorView, 0);
    assert(result);
}

void Window::hideKeyboard()
{
    // NOTE: ANativeActivity_hideSoftInput(app->activity, 0); does not work. @see https://stackoverflow.com/questions/5864790/how-to-show-the-soft-keyboard-on-native-activity
    auto [classInputMethodManager, inputMethodManager, decorView] = getKeyboardJni();

    // Runs lWindow.getViewToken()
    jclass classView = jniUtils::jniEnv->FindClass("android/view/View");
    assert(classView);
    jmethodID methodGetWindowToken = jniUtils::jniEnv->GetMethodID(classView, "getWindowToken", "()Landroid/os/IBinder;");
    assert(methodGetWindowToken);
    jobject binder = jniUtils::jniEnv->CallObjectMethod(decorView, methodGetWindowToken);
    assert(binder);

    // lInputMethodManager.hideSoftInput(...).
    jmethodID methodHideSoftInput = jniUtils::jniEnv->GetMethodID(classInputMethodManager, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
    assert(methodHideSoftInput);
    jboolean result = jniUtils::jniEnv->CallBooleanMethod(inputMethodManager, methodHideSoftInput, binder, 0);
    assert(result);
}

KeyboardInput& Window::getKeyboardInput()
{
    return keyboardInput;
}

Window::KeyboardJni Window::getKeyboardJni() const
{
    jclass classNativeActivity = jniUtils::jniEnv->GetObjectClass(jniUtils::activity);

    // Retrieves Context.INPUT_METHOD_SERVICE.
    jclass classContext = jniUtils::jniEnv->FindClass("android/content/Context");
    assert(classContext);
    jfieldID fieldINPUT_METHOD_SERVICE = jniUtils::jniEnv->GetStaticFieldID(classContext, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
    assert(fieldINPUT_METHOD_SERVICE);
    jobject INPUT_METHOD_SERVICE = jniUtils::jniEnv->GetStaticObjectField(classContext, fieldINPUT_METHOD_SERVICE);
    assert(INPUT_METHOD_SERVICE);

    // Runs getSystemService(Context.INPUT_METHOD_SERVICE).
    jclass classInputMethodManager = jniUtils::jniEnv->FindClass("android/view/inputmethod/InputMethodManager");
    assert(classInputMethodManager);
    jmethodID methodGetSystemService = jniUtils::jniEnv->GetMethodID(classNativeActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    assert(methodGetSystemService);
    jobject inputMethodManager = jniUtils::jniEnv->CallObjectMethod(jniUtils::activity, methodGetSystemService, INPUT_METHOD_SERVICE);
    assert(inputMethodManager);

    // Runs getWindow().getDecorView().
    jmethodID methodGetWindow = jniUtils::jniEnv->GetMethodID(classNativeActivity, "getWindow", "()Landroid/view/Window;");
    assert(methodGetWindow);
    jobject window = jniUtils::jniEnv->CallObjectMethod(jniUtils::activity, methodGetWindow);
    assert(window);
    jclass classWindow = jniUtils::jniEnv->FindClass("android/view/Window");
    assert(classWindow);
    jmethodID methodGetDecorView = jniUtils::jniEnv->GetMethodID(classWindow, "getDecorView", "()Landroid/view/View;");
    assert(methodGetDecorView);
    jobject decorView = jniUtils::jniEnv->CallObjectMethod(window, methodGetDecorView);
    assert(decorView);

    return {classInputMethodManager, inputMethodManager, decorView};
}
