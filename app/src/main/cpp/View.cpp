#include "View.h"

View::View(int renderWidth, int renderHeight, int screenWidth, int screenHeight, View::ViewportType viewportType)
    : width(renderWidth), height(renderHeight)
{
	//NOTE: For ViewportType::FIT, but also needed for extend!
	float ratioScreen = (float)screenWidth / (float)screenHeight;
	float ratioGame = (float)renderWidth / (float)renderHeight;
	if (ratioScreen > ratioGame)
	{
		viewportWidth = (int)((float)screenHeight * ratioGame);
		viewportHeight = screenHeight;
	}
	else
	{
		viewportWidth = screenWidth;
		viewportHeight = (int)((float)screenWidth / ratioGame);
	}

	if (viewportType == ViewportType::EXTEND)
	{
		if (viewportWidth == screenWidth)
		{
			float remainingSpace = screenHeight - viewportHeight;
			height += (int) (remainingSpace * ((float)renderHeight / (float)viewportHeight));
			viewportHeight = screenHeight;
		}
		else if (viewportHeight == screenHeight)
		{
			float remainingSpace = screenWidth - viewportWidth;
			width += (int) (remainingSpace * ((float)renderWidth / (float)viewportWidth));
			viewportWidth = screenWidth;
		}
		else
			InvalidCodePath;
	}

	center = { width / 2.0f, height / 2.0f };
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

void View::setSize(const Vector2i & size)
{
	width = size.x;
	height = size.y;
	shouldUpdate = true;
}

void View::setSize(int widthIn, int heightIn)
{
	width = widthIn;
	height = heightIn;
	shouldUpdate = true;
}

Vector2i View::getSize() const
{
	return { width, height };
}

const Vector2f & View::getCenter() const
{
	return center;
}

Mat4x4 View::getOrthoProj(const Vector2f scale) const
{
	assert(*this);

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

Vector2i View::getViewportSize() const
{
    return { viewportWidth, viewportHeight };
}
