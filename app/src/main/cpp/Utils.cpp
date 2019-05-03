#include "Utils.h"
#include "Types.h"

static uint counter = 0;

unsigned int utils::getGUID()
{
	return counter++;
}

float utils::lerp(float v0, float v1, float t)
{
	return (1 - t) * v0 + t * v1;
}

float utils::degreesToRadians(float degree)
{
	float radians = PiOver180 * degree;
	return radians;
}

float utils::radiansToDegrees(float radians)
{
	float degrees = _180OverPi * radians;
	return degrees;
}

void utils::logF(const char * string, ...)
{
#ifdef DEBUG
	va_list argList = { 0 };
	va_start(argList, string);
	__android_log_vprint(ANDROID_LOG_INFO, "utilsLog", string, argList);
	va_end(argList);
#endif
}

void utils::logFBreak(const char * string, ...)
{
#ifdef DEBUG
	va_list argList = { 0 };
	va_start(argList, string);
	__android_log_vprint(ANDROID_LOG_INFO, "utilsLog", string, argList);
	va_end(argList);

	assert(false);
#endif
}

void utils::log(const char * string)
{
#ifdef DEBUG
	__android_log_write(ANDROID_LOG_INFO, "utilsLog", string);
#endif
}

void utils::logBreak(const char * string)
{
#ifdef DEBUG
	do { log((string)); assert(false); } while (false);
#endif
}
