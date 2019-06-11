#pragma once

#include "Vector2.h"

struct TouchInput
{
	struct TouchInputStruct
	{
		Vector2f touchPos = { 0.0f, 0.0f };
		bool down = false;
		bool up = false;
		bool move = false;
	} inputs[2];
};