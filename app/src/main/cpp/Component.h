#pragma once

#include "EventManager.h"
#include "Utils.h"
#include "AssetManager.h"

class Component
{
protected:
	const uint32_t id;
public:
	Component(uint32_t id) : id(id) {}

	virtual void update(float dt, class Actor* owner) {}
	virtual void render() {}
	uint32_t getId() const { return id; };

	virtual ~Component() = default;
};