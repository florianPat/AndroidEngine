#include "RectangleShape.h"

void RectangleShape::setSize(const Vector2f & sizeIn)
{
	size = sizeIn;
}

void RectangleShape::setSize(float sizeX, float sizeY)
{
	size = { sizeX, sizeY };
}

void RectangleShape::setPosition(const Vector2f & position)
{
	pos = position;
}

void RectangleShape::setPosition(float x, float y)
{
	pos = { x, y };
}

void RectangleShape::setFillColor(const Color & color)
{
	fillColor = color;
}

void RectangleShape::setOrigin(const Vector2f & origin)
{
	org = origin;
}

void RectangleShape::setOrigin(float orgX, float orgY)
{
	org = { orgX, orgY };
}

void RectangleShape::setRotation(float rotation)
{
	rot = rotation;
}

Mat4x4 RectangleShape::getTransform() const
{
	Mat4x4 result = Mat4x4::identity();

	result *= Mat4x4::translate(pos);

	result *= Mat4x4::rotate(rot);

	result *= Mat4x4::translate(-org);

	result *= Mat4x4::scale({ getWidth(), getHeight() });

	return result;
}

const Color & RectangleShape::getFillColor() const
{
	return fillColor;
}

float RectangleShape::getWidth() const
{
	return size.x;
}

float RectangleShape::getHeight() const
{
	return size.y;
}

const Vector2f & RectangleShape::getSize() const
{
	return size;
}

const Vector2f & RectangleShape::getPosition() const
{
	return pos;
}

const Vector2f & RectangleShape::getOrigin() const
{
	return org;
}

float RectangleShape::getRotation() const
{
	return rot;
}
