#pragma once

#include "android_native_app_glue.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include "Clock.h"
#include "Sprite.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexLayout.h"
#include "Shader.h"
#include "Mat4x4.h"
#include "AssetManager.h"
#include "RectangleShape.h"
#include <memory>
#include "CircleShape.h"
#include "View.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "Sound.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Font.h"


class RenderWindow
{
public:
	enum class ViewportType
	{
		FIT,
		EXTEND
	};
private:
	android_app* app = nullptr;
	bool gainedFocus = false;
	bool resumed = false;
	bool validNativeWindow = false;
	bool initFinished = false;
	bool running = true;
	Clock clock;

	int32_t renderWidth = 0, renderHeight = 0;
	int screenWidth = 0, screenHeight = 0;
	int viewportWidth = 0, viewportHeight = 0;
	EGLDisplay display = EGL_NO_DISPLAY;
	EGLSurface surface = EGL_NO_SURFACE;
	EGLContext context = EGL_NO_CONTEXT;
	AssetManager assetManager;
	Shader shaderSprite;
	Shader shaderRectShape;
	View view;
	Mat4x4 orhtoProj;
	ViewportType viewportType;
	bool glContextLost = false;
	
	SLObjectItf engineObj = 0;
	SLEngineItf engine = 0;
	SLObjectItf outputMix = 0;

	SLObjectItf playerObj = 0;
	SLPlayItf player = 0;
	SLAndroidSimpleBufferQueueItf playerBuffer = 0;
	FT_Library fontLibrary = nullptr;
public:
	RenderWindow(android_app* app, int width, int height, ViewportType viewportType);
	RenderWindow(const RenderWindow& other) = delete;
	RenderWindow(RenderWindow&& other) = delete;
	RenderWindow& operator=(const RenderWindow& rhs) = delete;
	RenderWindow& operator=(RenderWindow&& rhs) = delete;
	bool processEvents();

	void clear();
	void close();
	void draw(const Sprite& sprite);
	void draw(const RectangleShape& rect);
	void draw(const CircleShape& circle);
	void render();
	AssetManager* getAssetManager();
	View& getDefaultView();
	int getViewportWidth() const;
	int getViewportHeight() const;
	int getRenderWidth() const;
	int getRenderHeight() const;
	void recoverFromContextLoss();
	Clock& getClock() const;
	void play(const Sound* snd);
	Shader* getSpriteShader();
	FT_Library getFontLibrary();
private:
	void deactivate();
	void processAppEvent(int32_t command);
	static void AppEventCallback(android_app* app, int32_t command);
	int processInputEvent(AInputEvent* event);
	static int InputEventCallback(android_app* app, AInputEvent* event);
	bool startGfx();
	bool initDisplay(EGLConfig& config);
	bool initSurface(EGLConfig& config);
	bool initContext(EGLConfig& config);
	void stopGfx();
	void getAndSetTouchInputPos(AInputEvent* event);
	bool startSnd();
	void stopSnd();
	void setupGfxGpu();
	void checkIfToRecoverFromContextLoss();
	bool startFont();
	void stopFont();
};