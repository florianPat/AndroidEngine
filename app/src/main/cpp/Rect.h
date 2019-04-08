#pragma once

#include "Vector2.h"

template <typename T>
class Rect
{
public:
	inline Rect() = default;
	inline Rect(T left, T top, T width, T height) : top(top), left(left), width(width), height(height) {}
	//Rect(const Rect& rect) : top(rect.top), left(rect.left), width(rect.width), height(rect.height) {}
	template <typename T2>
	inline operator Rect<T2>() const
	{
		return { (T2)left,(T2)top,(T2)width,(T2)height };
	}
	inline bool intersects(const Rect& rect) const
	{
		return top < rect.getBottom() && getBottom() > rect.top &&
			left < rect.getRight() && getRight() > rect.left;
	}
	template <typename T2>
	inline bool contains(Vector2<T2> p) const
	{
		return p.y >= top && p.y <= getBottom() && p.x >= left && p.x <= getRight();
	}
	template <typename T2>
	inline bool contains(Rect<T2> p) const
	{
		return p.top >= top && p.getBottom() <= getBottom() && p.left >= left && p.getRight() <= getRight();
	}
	inline T getBottom() const
	{
		return top + height;
	}
	inline T getRight() const
	{
		return left + width;
	}
public:
	T top;
	T left;
	T width;
	T height;
};

typedef Rect<int> IntRect;
typedef Rect<float> FloatRect;