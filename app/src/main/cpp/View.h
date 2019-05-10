#pragma once

#include "Vector2.h"
#include "Mat4x4.h"

class View
{
	float width = 0;
	float height = 0;
	Vector2f center = { 0.0f, 0.0f };
	bool shouldUpdate = false;
	//TODO: Implement
	/*float rot = 0.0f;
	float scl = 1.0f;*/
public:
	View(float width, float height);
	Mat4x4 getOrthoProj(const Vector2f scale = { 1.0f, 1.0f }) const;
	bool updated();
public:
	View() = delete;
	void setCenter(const Vector2f& center);
	void setCenter(float x, float y);
	void setSize(const Vector2f& size);
	void setSize(float width, float height);
	Vector2f getSize() const;
	const Vector2f& getCenter() const;
};