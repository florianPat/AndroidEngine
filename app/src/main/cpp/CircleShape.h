#pragma once

#include "Vector2.h"
#include "Color.h"
#include "Mat4x4.h"

class CircleShape
{
	Vector2f pos = { 0.0f, 0.0f };
	float rad = 1.0f;
	Color color = Colors::White;
public:
	CircleShape() = default;
	void setPosition(const Vector2f& position);
	void setPosition(float x, float y);
	void setRadius(float radius);
	void setFillColor(const Color& color);
	const Vector2f& getPosition() const;
	float getRadius() const;
	const Color& getFillColor() const;
	Mat4x4 getTransform() const;
};