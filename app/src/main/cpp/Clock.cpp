#include "Clock.h"
#include "stdlib.h"
#include "Utils.h"
#include <time.h>

Clock::Clock()
{
	srand((uint32_t)time(nullptr));
	restart();
}

void Clock::restart()
{
	elapsed = 0;
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

uint64_t Clock::now()
{
	timespec timeVal = { 0 };
	if (clock_gettime(CLOCK_REALTIME, &timeVal) != 0)
	{
		utils::logBreak("clock_gettime error!");
	}
	return (uint64_t)timeVal.tv_sec * 1000000000ll + timeVal.tv_nsec;
}

void Clock::update()
{
	uint64_t currentTime = now();
	elapsed = currentTime - lastTime;
	elapsedTotal = currentTime - firstTime;
	lastTime = currentTime;
}
