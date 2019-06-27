#pragma once

#include "Actor.h"

class GameObjectManager
{
	static constexpr uint32_t DEFAULT_COMPONENT_SIZE = 512;
	Vector<Actor> actors;
	Vector<std::pair<int, int>> layers[4];
	Vector<int> destroyActorQueue;
public:
	GameObjectManager();
	Actor* addActor();
	Actor* addActor(uint32_t componentsSize);
	template <typename T, typename... Args>
	void addUpdateComponent(Args&&... args);
	template <typename T, typename... Args>
	void addComponent(uint renderLayer, Args&&... args);
	template <typename T, typename... Args>
	void addRenderComponent(uint renderLayer, Args&&... args);
	void destroyActor(uint32_t actorId);
	void updateAndDrawActors(float dt);
private:
	void destroyActors();
};

template<typename T, typename... Args>
inline void GameObjectManager::addUpdateComponent(Args&&... args)
{
	Actor& actor = actors.back();
	actor.addComponent<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
inline void GameObjectManager::addComponent(uint renderLayer, Args&&... args)
{
	Actor& actor = actors.back();
    const T* component = actor.addComponent<T>(std::forward<Args>(args)...);

	layers[renderLayer].push_back(std::make_pair(actors.size() - 1, actor.getComponentIndex(component->getId())));
}

template<typename T, typename... Args>
inline void GameObjectManager::addRenderComponent(uint renderLayer, Args&&... args)
{
	Actor& actor = actors.front();
    const T* component = actor.addComponent<T>(std::forward<Args>(args)...);

	layers[renderLayer].push_back(std::make_pair(0, actor.getComponentIndex(component->getId())));
}