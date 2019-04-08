#include "CircleShape.h"

void CircleShape::setPosition(const Vector2f & position)
{
	pos = position;
}

void CircleShape::setPosition(float x, float y)
{
	pos = { x, y };
}

void CircleShape::setRadius(float radius)
{
	rad = radius;
}

void CircleShape::setFillColor(const Color & colorIn)
{
	color = colorIn;
}

const Vector2f & CircleShape::getPosition() const
{
	return pos;
}

float CircleShape::getRadius() const
{
	return rad;
}

const Color & CircleShape::getFillColor() const
{
	return color;
}

Mat4x4 CircleShape::getTransform() const
{
	Mat4x4 result = Mat4x4::identity();

	result *= Mat4x4::translate(pos);

	return result;
}
