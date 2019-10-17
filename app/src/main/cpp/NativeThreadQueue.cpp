#include "NativeThreadQueue.h"
#include <unistd.h>

const int32_t NativeThreadQueue::nThreads = 4;//sysconf(_SC_NPROCESSORS_ONLN) - 1;

NativeThreadQueue::NativeThreadQueue() : entries(SIZE), nativeThreads(nThreads)
{
}

void NativeThreadQueue::startThreads()
{
    int32_t result = sem_init(&semaphore, 0, 0);
    assert(result == 0);

    utils::logF("nThreads: %i", nThreads);
    for(uint32_t i = 0; i < nThreads; ++i)
    {
        nativeThreads.emplace_back(GET_DELEGATE_WITH_PARAM_FORM(void(volatile bool&, void*),
                                                                NativeThreadQueue, &NativeThreadQueue::threadFunc), this);
    }
}

void NativeThreadQueue::endThreads()
{
    nativeThreads.clear();

    int32_t result = 0;
    for(int32_t i = 0; i < nThreads; ++i)
    {
        result = sem_post(&semaphore);
        assert(result == 0);
    }

    result = sem_destroy(&semaphore);
    assert(result == 0);
}

void NativeThreadQueue::threadFunc(volatile bool& running, void* arg)
{
    NativeThreadQueue* queue = (NativeThreadQueue*) arg;
    while(running)
    {
        uint32_t originalNextEntryIndex = queue->nextEntryIndex;
        if(originalNextEntryIndex < queue->size)
        {
            bool written = __sync_bool_compare_and_swap(&queue->nextEntryIndex, originalNextEntryIndex, originalNextEntryIndex + 1);
            if(written)
            {
                Entry entry = queue->entries[originalNextEntryIndex];
                entry.delegate(entry.data, queue->flushArg);
            }
        }
        else
        {
            int32_t result = sem_wait(&semaphore);
            assert(result == 0);
        }
    }
}

void NativeThreadQueue::addEntry(Delegate<void(uint32_t, float)>&& delegate, uint32_t param)
{
    assert(entryCount < SIZE);

    entries[entryCount].delegate = delegate;
    entries[entryCount].data = param;

    __sync_synchronize();

    ++entryCount;
}

void NativeThreadQueue::flushToWithAndReset(uint32_t index, float arg)
{
#ifdef DEBUG
    if(!startedFlushing)
        startedFlushing = true;
#endif

    assert(index <= entryCount);
    flushArg = arg;

    __sync_synchronize();

    size = index;
    nextEntryIndex = 0;

    __sync_synchronize();

    uint32_t nEntriesToDo = size - nextEntryIndex;
    int32_t result = 0;
    if(nEntriesToDo > 1)
    {
        nEntriesToDo = (nEntriesToDo > nThreads) ? nThreads : nEntriesToDo;
        for(uint32_t i = 0; i < nEntriesToDo; ++i)
        {
            result = sem_post(&semaphore);
            assert(result == 0);
        }
    }
    while(getAndCompleteNextEntry());
}

void NativeThreadQueue::flushToWith(uint32_t index, float arg)
{
#ifdef DEBUG
    if(!startedFlushing)
        startedFlushing = true;
#endif

    assert(index <= entryCount);
    flushArg = arg;

    __sync_synchronize();

    size = index;

    __sync_synchronize();

    uint32_t nEntriesToDo = size - nextEntryIndex;
    int32_t result = 0;
    if(nEntriesToDo > 1)
    {
        nEntriesToDo = (nEntriesToDo > nThreads) ? nThreads : nEntriesToDo;
        for(uint32_t i = 0; i < nEntriesToDo; ++i)
        {
            result = sem_post(&semaphore);
            assert(result == 0);
        }
    }
    while(getAndCompleteNextEntry());
}

void NativeThreadQueue::flush()
{
#ifdef DEBUG
    if(!startedFlushing)
        startedFlushing = true;
#endif

    flushArg = 0.0f;

    __sync_synchronize();

    size = entryCount;

    __sync_synchronize();

    uint32_t nEntriesToDo = size - nextEntryIndex;
    int32_t result = 0;
    if(nEntriesToDo > 1)
    {
        nEntriesToDo = (nEntriesToDo > nThreads) ? nThreads : nEntriesToDo;
        for(uint32_t i = 0; i < nEntriesToDo; ++i)
        {
            result = sem_post(&semaphore);
            assert(result == 0);
        }
    }
    while(getAndCompleteNextEntry());
}

void NativeThreadQueue::setNextWriteIndex(uint32_t newWriteIndex)
{
    assert((size < newWriteIndex) || (size == nextEntryIndex));

    entryCount = newWriteIndex;
}

#ifdef DEBUG
bool NativeThreadQueue::getStartedFlushing() const
{
    return startedFlushing;
}

void NativeThreadQueue::resetStartedFlushing()
{
    startedFlushing = false;
}
#endif

bool NativeThreadQueue::getAndCompleteNextEntry()
{
    bool result = false;

    uint32_t originalNextEntryIndex = nextEntryIndex;
    if(originalNextEntryIndex < size)
    {
        bool written = __sync_bool_compare_and_swap(&nextEntryIndex, originalNextEntryIndex, originalNextEntryIndex + 1);
        if(written)
        {
            Entry entry = entries[originalNextEntryIndex];
            entry.delegate(entry.data, flushArg);
            result = true;
        }
    }

    return result;
}
