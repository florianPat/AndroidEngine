#pragma once

#include "Vector2.h"
#include "Mat4x4.h"

class View
{
	int32_t width = 0;
	int32_t height = 0;
	int32_t viewportWidth = 0, viewportHeight = 0;
	Vector2f center = { 0.0f, 0.0f };
	bool shouldUpdate = false;
	//TODO: Implement
	/*float rot = 0.0f;
	float scl = 1.0f;*/
public:
	enum class ViewportType
	{
		FIT,
		EXTEND
	};
public:
	View() = default;
	View(int32_t renderWidth, int32_t renderHeight, int32_t screenWidth, int32_t screenHeight, View::ViewportType viewportType);
	Mat4x4 getOrthoProj(const Vector2f scale = { 1.0f, 1.0f }) const;
	bool updated();
	void setCenter(const Vector2f& center);
	void setCenter(float x, float y);
	void setSize(const Vector2i& size);
	void setSize(int32_t width, int32_t height);
	Vector2i getSize() const;
	Vector2i getViewportSize() const;
	const Vector2f& getCenter() const;
	explicit operator bool () const { return (viewportWidth != 0); }
};