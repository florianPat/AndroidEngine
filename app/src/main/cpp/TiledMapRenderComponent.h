#pragma once

#include "Component.h"
#include "Sprite.h"
#include "Window.h"
#include "Utils.h"

class TiledMapRenderComponent : public Component
{
	const Sprite sprite;
	Graphics& gfx;
public:
	TiledMapRenderComponent(Sprite& sprite, Graphics& gfx)
		: Component(utils::getGUID()), sprite(sprite),
		  gfx(gfx)
		  {}
	void render() override { gfx.draw(sprite); }
};