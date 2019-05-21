#pragma once

#include "Vector2.h"

struct TouchInput
{
	struct TouchInputStruct
	{
		Vector2f touchPos;
		bool down;
		bool up;
		bool move;
	} inputs[2];
};