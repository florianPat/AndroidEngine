#pragma once

#include "Component.h"
#include "Sprite.h"
#include "RenderWindow.h"
#include "Utils.h"

class TiledMapRenderComponent : public Component
{
	const Sprite sprite;
	RenderWindow& renderTarget;
public:
	TiledMapRenderComponent(Sprite& sprite, RenderWindow& renderTarget, EventManager& em, Actor* owner) : Component(utils::getGUID(), &em, renderTarget.getAssetManager(), owner, Component::instantiateFunc<TiledMapRenderComponent>()), sprite(sprite), renderTarget(renderTarget) {};
	void updateAndRender(float dt) { renderTarget.draw(sprite); };
	gomSort::SortKey sort() override { return gomSort::SortKey{ 0, sprite.getPosition().y }; }
};