#pragma once

#include <android/log.h>

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
	unsigned int getGUID();
	float lerp(float v0, float v1, float t);
	float degreesToRadians(float degree);
	float radiansToDegrees(float radians);
	template<typename... Args>
	void logF(const char* string, Args&&... args)
    {
#ifdef DEBUG
        __android_log_print(ANDROID_LOG_INFO, "utilsLog", string, args...);
#endif
    }
    template<typename... Args>
	void logFBreak(const char* string, Args&&... args)
    {
#ifdef DEBUG
        __android_log_print(ANDROID_LOG_INFO, "utilsLog", string, args...);
        InvalidCodePath;
#endif
    }
	void log(const char* string);
	void logBreak(const char* string);
}