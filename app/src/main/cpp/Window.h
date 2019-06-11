#pragma once

#include "android_native_app_glue.h"
#include "Clock.h"
#include "AssetManager.h"
#include <memory>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "Sound.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "GraphicsOGL2.h"
#include "TouchInput.h"

#define Graphics GraphicsOGL2

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

	SLObjectItf engineObj = 0;
	SLEngineItf engine = 0;
	SLObjectItf outputMix = 0;
	SLObjectItf playerObj = 0;
	SLPlayItf player = 0;
	SLAndroidSimpleBufferQueueItf playerBuffer = 0;
public:
	Window(android_app* app, int width, int height, View::ViewportType viewportType);
	Window(const Window& other) = delete;
	Window(Window&& other) = delete;
	Window& operator=(const Window& rhs) = delete;
	Window& operator=(Window&& rhs) = delete;
	bool processEvents();

	void close();
	AssetManager* getAssetManager();
	Clock& getClock();
	void play(const Sound* snd);
	FT_Library getFontLibrary();
    Graphics& getGfx();
    const TouchInput& getTouchInput() const;
private:
	void deactivate();
	void processAppEvent(int32_t command);
	static void AppEventCallback(android_app* app, int32_t command);
	int processInputEvent(AInputEvent* event);
	static int InputEventCallback(android_app* app, AInputEvent* event);
	void getAndSetTouchInputPos(AInputEvent* event, int pointerId, uint pointerIndex);
	bool startSnd();
	void stopSnd();
	bool startFont();
	void stopFont();
	void checkAndRecoverFromContextLoss();
};