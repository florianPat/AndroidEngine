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
		: Component(utils::getGUID(),
		  Component::instantiateFunc<TiledMapRenderComponent>()), sprite(sprite),
		  gfx(gfx) {};
	void updateAndRender(float dt) { gfx.draw(sprite); };
	gomSort::SortKey sort() override { return gomSort::SortKey{ 0, sprite.pos.y }; }
};