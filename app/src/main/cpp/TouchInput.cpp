#include "TouchInput.h"

Vector2f TouchInput::touchPos = { 0.0f, 0.0f };
bool TouchInput::touched = false;
bool TouchInput::shouldUp = false;

bool TouchInput::isTouched()
{
	return touched;
}

const Vector2f & TouchInput::getPosition()
{
	return touchPos;
}

void TouchInput::setPosition(float x, float y)
{
	touchPos = { x, y };
}

void TouchInput::setTouched(bool touch)
{
	touched = touch;
}

void TouchInput::setShouldUp(bool shouldUpIn)
{
	shouldUp = shouldUpIn;
}

bool TouchInput::getShouldUp()
{
	return shouldUp;
}
