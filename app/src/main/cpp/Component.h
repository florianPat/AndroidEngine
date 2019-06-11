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
	const uint id;
	void(*updateAndDrawFunc)(char* thiz, float dt);
public:
	Component(uint id, UpdateAndDrawFunc updateAndDrawFunc) : id(id),
												 	 		 updateAndDrawFunc(updateAndDrawFunc)
	{};

	void updateAndDraw(float dt) { updateAndDrawFunc((char*)this, dt); };
	virtual gomSort::SortKey sort() { return gomSort::SortKey{ 0, 0.0f }; };
	uint getId() { return id; };

	//NOTE: The current working of this makes it necessary to only derive once from Component
	template <typename T>
	static UpdateAndDrawFunc instantiateFunc()
	{
		return [](char* thiz, float dt) {T* p = (T*)thiz; p->updateAndRender(dt); };
	}
	virtual ~Component() = default;
};