#include "View.h"

View::View(float width, float height) : width(width), height(height), center(width / 2.0f, height / 2.0f)
{
}

void View::setCenter(const Vector2f & centerIn)
{
	center = centerIn;
	shouldUpdate = true;
}

void View::setCenter(float x, float y)
{
	center = { x, y };
	shouldUpdate = true;
}

void View::setSize(const Vector2f & size)
{
	width = size.x;
	height = size.y;
	shouldUpdate = true;
}

void View::setSize(float widthIn, float heightIn)
{
	width = widthIn;
	height = heightIn;
	shouldUpdate = true;
}

Vector2f View::getSize() const
{
	return { width, height };
}

const Vector2f & View::getCenter() const
{
	return center;
}

Mat4x4 View::getOrthoProj(const Vector2f scale) const
{
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	Mat4x4 result = Mat4x4::orthoProj(-1.0f, 1.0f, center.x - halfWidth, center.y - halfHeight, center.x + halfWidth, center.y + halfHeight);
	result.scale(scale);
	return result;
}

bool View::updated()
{
	bool result = shouldUpdate;
	shouldUpdate = false;
	return result;
}
