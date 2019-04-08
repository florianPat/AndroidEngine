#pragma once

#include <android/log.h>

#undef assert

#ifndef DEBUG
#define assert(exp) if(!((exp) && true)) __android_log_assert(nullptr, "ASSERT", #exp);
#else
//NOTE: I do this so that the expression in the assert gets executed, because sometimes I have "critical" code in there (which may
// is a bad idea)
#define assert(exp) if(!((exp) && true)) utils::log("assert would have fired now!");
#endif

#define InvalidCodePath assert(!"InvalidCodePath")

#define arrayCount(x) sizeof(x) / sizeof(x[0])

#define Kilobyte(x) x * 1024ll
#define Megabyte(x) Kilobyte(x) * 1024ll
#define Gigabyte(x) Megabyte(x) * 1024ll

static constexpr float PIf = 3.1415927f;

static constexpr float PiOver180 = PIf / 180;
static constexpr float _180OverPi = 180 / PIf;

namespace utils
{
	unsigned int getGUID();
	float lerp(float v0, float v1, float t);
	float degreesToRadians(float degree);
	float radiansToDegrees(float radians);
	void logF(const char* string, ...);
	void logFBreak(const char* string, ...);
	void log(const char* string);
	void logBreak(const char* string);
}