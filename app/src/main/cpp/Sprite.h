#pragma once

#include "Texture.h"
#include "Vector2.h"
#include "Rect.h"
#include "Mat4x4.h"
#include "Color.h"

class Sprite
{
	const Texture* texture = nullptr;
	IntRect rect;
	Vector2f pos = { 0.0f, 0.0f }, org = { 0.0f, 0.0f };
	Vector2f scl = { 1.0f, 1.0f };
	float rot = 0.0f;
	Color color = Colors::White;
public:
	Sprite() = default;
	Sprite(const Texture* texture);
	Sprite(const Texture* texture, const IntRect& rect);
	void setTexture(const Texture* texture, bool resetRect = false);
	//NOTE: Texture rect in texture widht/height "space". Not 0 - 1 space!!
	void setTextureRect(const IntRect& rect);
	const Texture* getTexture() const;
	//NOTE: Texture rect in texture widht/height "space". Not 0 - 1 space!!
	const IntRect& getTextureRect() const;
	FloatRect getGlobalBounds() const;
	Vector2f getSize() const;
	float getWidth() const;
	float getHeight() const;
	void setPosition(float x, float y);
	void setPosition(const Vector2f &position);
	void setRotation(float angle);
	void setScale(float factor);
	void setScale(float factorX, float factorY);
	void setScale(const Vector2f &factors);
	//NOTE: Origin in texture widht/height "space". Not 0 - 1 space!!
	void setOrigin(float x, float y);
	//NOTE: Origin in texture widht/height "space". Not 0 - 1 space!!
	void setOrigin(const Vector2f &origin);
	const Vector2f& getPosition() const;
	float getRotation() const;
	const Vector2f& getScale() const;
	const Vector2f& getOrigin() const;
	const Mat4x4 getTransform() const;
	void setColor(const Color& c);
	const Color& getColor() const;
};