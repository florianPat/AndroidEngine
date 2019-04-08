#pragma once

#include "Types.h"

class Color
{
public:
	uchar r = 0;
	uchar g = 0;
	uchar b = 0;
	uchar a = 255;
public:
	Color() = default;
	Color(uchar r, uchar g, uchar b, uchar a = 255) : r(r), g(g), b(b), a(a) {}
	Color(uint c) : r(c & 0xFF), g((c >> 8) & 0xFF), b((c >> 16) & 0xFF), a((c >> 24) & 0xFF) {}
	uint toInteger() const
	{
		uint result = 0;

		result = (((uint)r) | (((uint)g) << 8) | (((uint)b) << 16) | (((uint)a) << 24));

		return result;
	}

	bool operator==(const Color& rhs)
	{
		return toInteger() == rhs.toInteger();
	}
	bool operator!=(const Color& rhs)
	{
		return toInteger() != rhs.toInteger();
	}
	
	//Clamp!
	/*Color operator+(const Color& rhs)
	{
		return Color(toInteger() + rhs.toInteger());
	}
	Color& operator+=(const Color& rhs)
	{
		*this = Color(toInteger() + rhs.toInteger());
		return *this;
	}

	Color operator-(const Color& rhs)
	{
		return Color(toInteger() - rhs.toInteger());
	}
	Color& operator-=(const Color& rhs)
	{
		*this = Color(toInteger() - rhs.toInteger());
		return *this;
	}

	Color operator*(const Color& rhs)
	{
		return Color(toInteger() * rhs.toInteger());
	}
	Color& operator*=(const Color& rhs)
	{
		*this = Color(toInteger() * rhs.toInteger());
		return *this;
	}*/
};

namespace Colors
{
	static const Color Black = Color();
	static const Color White = Color(255, 255, 255);
	static const Color Red = Color(255, 0, 0);
	static const Color Green = Color(0, 255, 0);
	static const Color Blue = Color(0, 0, 255);
	static const Color Yellow = Color(255, 255, 0);
	static const Color Magenta = Color(255, 0, 255);
	static const Color Cyan = Color(0, 255, 255);
	static const Color Transparent = Color(0, 0, 0, 0);
}