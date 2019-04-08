#include "Clock.h"
#include "stdlib.h"
#include "Utils.h"
#include <time.h>

Clock::Clock()
{
	srand(time(NULL));
	restart();
}

void Clock::restart()
{
	elapsed = 0.0f;
	firstTime = now();
	lastTime = firstTime;
}

Time Clock::getTime()
{
	update();

	return Time{ elapsed };
}

Time Clock::getElapsedTimeTotal()
{
	update();

	return Time{ elapsedTotal };
}

int64_t Clock::now()
{
	timespec timeVal;
	if (clock_gettime(CLOCK_REALTIME, &timeVal) != 0)
	{
		utils::logBreak("clock_gettime error!");
	}
	return timeVal.tv_sec * 1000000000ll + timeVal.tv_nsec;
}

void Clock::update()
{
	double currentTime = now();
	elapsed = currentTime - lastTime;
	elapsedTotal = currentTime - firstTime;
	lastTime = currentTime;
}
