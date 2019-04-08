#pragma once

#include "Vector2.h"
#include "Color.h"
#include "Mat4x4.h"

class RectangleShape
{
	Vector2f size = { 1.0f, 1.0f };
	Vector2f pos = { 0.0f, 0.0f };
	Color fillColor = Colors::White;
	Vector2f org = { 0.0f, 0.0f };
	float rot = 0.0f;
public:
	RectangleShape() = default;
	void setSize(const Vector2f& size);
	void setSize(float sizeX, float sizeY);
	void setPosition(const Vector2f& position);
	void setPosition(float x, float y);
	void setFillColor(const Color& color);
	void setOrigin(const Vector2f& origin);
	void setOrigin(float orgX, float orgY);
	void setRotation(float rotation);
	Mat4x4 getTransform() const;
	const Color& getFillColor() const;
	float getWidth() const;
	float getHeight() const;
	const Vector2f& getSize() const;
	const Vector2f& getPosition() const;
	const Vector2f& getOrigin() const;
	float getRotation() const;
};