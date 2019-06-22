#pragma once

#include "EventManager.h"
#include "Utils.h"
#include "AssetManager.h"

class Component
{
protected:
	const uint id;
public:
	Component(uint id) : id(id) {}

	virtual void update(float dt, class Actor* owner) {}
	virtual void render() {}
	uint getId() const { return id; };

	virtual ~Component() = default;
};