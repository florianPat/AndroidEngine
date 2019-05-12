#pragma once

#include "Vector2.h"

class TouchInput
{
	static Vector2f touchPos;
	static bool touched;
	static bool shouldUp;
public:
	static bool isTouched();
	static const Vector2f& getPosition();
private:
	friend class Window;
	static void setPosition(float x, float y);
	static void setTouched(bool touch);
	static void setShouldUp(bool shouldUp);
	static bool getShouldUp();
};