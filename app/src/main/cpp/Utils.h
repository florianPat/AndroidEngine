#pragma once

#include <android/log.h>
#include "Types.h"

#undef assert

#ifdef DEBUG
#define assert(exp) if(!((exp) && true)) __android_log_assert(nullptr, "ASSERT", #exp);
#else
#define assert(exp)
#endif

#define InvalidCodePath static_assert("InvalidCodePath")

#define arrayCount(x) sizeof(x) / sizeof(x[0])

#define Kilobyte(x) x * 1024ll
#define Megabyte(x) Kilobyte(x) * 1024ll
#define Gigabyte(x) Megabyte(x) * 1024ll

static constexpr float PIf = 3.1415927f;

static constexpr float PiOver180 = PIf / 180;
static constexpr float _180OverPi = 180 / PIf;

namespace utils
{
	uint32_t getGUID();
	float lerp(float v0, float v1, float t);
	float degreesToRadians(float degree);
	float radiansToDegrees(float radians);
	template<typename... Args>
	void logF(const char* string, Args&&... args)
    {
#ifdef DEBUG
        __android_log_print(ANDROID_LOG_DEBUG, "utilsLog", string, args...);
#endif
    }
    template<typename... Args>
	void logFBreak(const char* string, Args&&... args)
    {
#ifdef DEBUG
        __android_log_print(ANDROID_LOG_DEBUG, "utilsLog", string, args...);
        InvalidCodePath;
#endif
    }
	inline void log(const char* string)
	{
#ifdef DEBUG
		__android_log_write(ANDROID_LOG_DEBUG, "utilsLog", string);
#endif
	}
	inline void logBreak(const char* string)
	{
#ifdef DEBUG
		log((string));
		InvalidCodePath;
#endif
	}
}