#pragma once

#include "EventManager.h"
#include "gomSort.h"
#include "Utils.h"
#include "AssetManager.h"

class Component
{
protected:
	const uint id;
public:
	Component(uint id) : id(id)
	{};

	virtual void update(float dt, class Actor* owner) {}
	virtual void render() {}
	virtual gomSort::SortKey sort() { return gomSort::SortKey{ 0, 0.0f }; };
	uint getId() const { return id; };

	virtual ~Component() = default;
};