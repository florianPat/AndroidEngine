#include "GameObjectManager.h"
#include "Utils.h"

GameObjectManager::GameObjectManager(uint32_t renderActorSize) : actors(), pipelineIndexes(),
	nativeThreadQueue(Globals::window->getNativeThreadQueue())
{
    //NOTE: This is the RenderOnlyActor
	actors.push_back(Actor(actors.size(), renderActorSize));
}

Actor* GameObjectManager::addActor(uint32_t componentsSize)
{
	assert(!nativeThreadQueue.getStartedFlushing());

	actors.push_back(Actor(actors.size(), componentsSize));
	nativeThreadQueue.addEntry(GET_DELEGATE_WITH_PARAM_FORM(void(uint32_t, float),
	        GameObjectManager, &GameObjectManager::actorUpdateDelegate), actors.size() - 1);

	return &actors.back();
}

void GameObjectManager::endPipeline()
{
	assert(!nativeThreadQueue.getStartedFlushing());

	//NOTE: should be - 1 because the first one is reserved for the only draw components
	// But the first one is reserved for clearing the triggerEventVariableVector (see setup of Level)
	pipelineIndexes.push_back(actors.size());
}

void GameObjectManager::destroyActor(uint32_t actorId)
{
	//NOTE: Needs to be implemented in multithreading
	InvalidCodePath;
	//destroyActorQueue.push_back(actorId);
}

void GameObjectManager::updateAndDrawActors(float dt)
{
	nativeThreadQueue.flushToWithAndReset(1, 0.0f);
	for(auto it = pipelineIndexes.begin(); it != pipelineIndexes.end(); ++it)
	{
		nativeThreadQueue.flushToWith(*it, dt);
	}
	nativeThreadQueue.flush();

	for(int i = 0; i < arrayCount(layers); ++i)
	{
		auto& layer = layers[i];

		for(auto it = layer.begin(); it != layer.end(); ++it)
		{
			Actor& actor = actors[it->first];
			actor.getComponent<Component>(it->second)->render();
		}
	}
}

void GameObjectManager::actorUpdateDelegate(uint32_t specificArg, float broadArg)
{
	Actor* actor = &actors[specificArg];

	actor->update(broadArg);
}
