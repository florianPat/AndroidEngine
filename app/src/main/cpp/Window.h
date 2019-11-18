#pragma once

#include "android_native_app_glue.h"
#include "Clock.h"
#include "AssetManager.h"
#include "Sound.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "TouchInput.h"
#include "NativeThreadQueue.h"
#include "Audio.h"
#include "Graphics.h"

class Window
{
	android_app* app = nullptr;
	bool gainedFocus = false;
	bool resumed = false;
	bool validNativeWindow = false;
	bool initFinished = false;
	bool running = true;
	bool recreating = false;

    Clock clock;
    AssetManager assetManager;
    TouchInput touchInput;
    FT_Library fontLibrary = nullptr;

    Graphics gfx;

    NativeThreadQueue nativeThreadQueue;
    uint32_t threadQueueEventBeginning = 0;

    Audio audio;
public:
	Window(android_app* app, int32_t width, int32_t height, View::ViewportType viewportType);
	Window(const Window& other) = delete;
	Window(Window&& other) = delete;
	Window& operator=(const Window& rhs) = delete;
	Window& operator=(Window&& rhs) = delete;
	bool processEvents();

	void close();
	AssetManager* getAssetManager();
	Clock& getClock();
	FT_Library getFontLibrary();
    Graphics& getGfx();
    const TouchInput& getTouchInput() const;
    NativeThreadQueue& getNativeThreadQueue();
    Audio& getAudio();
    void callToGetEventJobsBeginning();
private:
	void deactivate();
	void processAppEvent(int32_t command);
	static void AppEventCallback(android_app* app, int32_t command);
	int processInputEvent(AInputEvent* event);
	static int32_t InputEventCallback(android_app* app, AInputEvent* event);
	void getAndSetTouchInputPos(AInputEvent* event, int32_t pointerId, uint pointerIndex);
	bool startFont();
	void stopFont();
	void checkAndRecoverFromContextLoss();
	void finishInit();
	void finishDeinit();
};