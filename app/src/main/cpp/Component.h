#pragma once

#include "EventManager.h"
#include "gomSort.h"
#include "Utils.h"
#include "AssetManager.h"

class Actor;

typedef void(*UpdateAndDrawFunc)(char* thiz, float dt);

class Component
{
protected:
	const int id;
	EventManager* eventManager;
	void(*updateAndDrawFunc)(char* thiz, float dt);
	AssetManager* assetManager;
	Actor* owner;
public:
	Component(int id, EventManager* eventManager, AssetManager* assetManager, Actor* owner, UpdateAndDrawFunc updateAndDrawFunc) : id(id),
																						eventManager(eventManager), updateAndDrawFunc(updateAndDrawFunc),
																						assetManager(assetManager), owner(owner) {};
	void updateAndDraw(float dt) { updateAndDrawFunc((char*)this, dt); };
	virtual gomSort::SortKey sort() { return gomSort::SortKey{ 0, 0.0f }; };
	int getId() { return id; };

	template <typename T>
	static UpdateAndDrawFunc instantiateFunc()
	{
		return [](char* thiz, float dt) {T* p = (T*)thiz; p->updateAndDraw(dt); };
	}
	virtual ~Component() = default;
};