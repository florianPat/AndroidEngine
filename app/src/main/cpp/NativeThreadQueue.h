#pragma once

#include <semaphore.h>
#include "NativeThread.h"

class NativeThreadQueue
{
    struct Entry
    {
        uint32_t data;
        Delegate<void(uint32_t, float)> delegate;
    };

    static constexpr int32_t SIZE = 16;
    Vector<Entry> entries;
    volatile uint32_t entryCount = 0;
    volatile uint32_t nextEntryIndex = 0;
    volatile uint32_t size = 0;
    float flushArg = 0.0f;
    sem_t semaphore;
    static const int32_t nThreads;
    HeapArray<NativeThread> nativeThreads;
#ifdef DEBUG
    bool startedFlushing = false;
#endif
public:
    NativeThreadQueue();
    void startThreads();
    void endThreads();
    NativeThreadQueue(const NativeThreadQueue& other) = delete;
    NativeThreadQueue& operator=(const NativeThreadQueue& rhs) = delete;
    NativeThreadQueue(NativeThreadQueue&& other) = delete;
    NativeThreadQueue& operator=(NativeThreadQueue&& rhs) = delete;
    void addEntry(Delegate<void(uint32_t, float)>&& delegate, uint32_t param);
    void setNextWriteIndex(uint32_t newWriteIndex);
    void flush(float arg = 0.0f);
    void flushToWithAndReset(uint32_t index, float arg = 0.0f);
    void flushToWith(uint32_t index, float arg = 0.0f);
    void flushFrom(uint32_t index, float arg = 0.0f);
    uint32_t getSize() const;
#ifdef DEBUG
    bool getStartedFlushing() const;
    void resetStartedFlushing();
#endif
private:
    void threadFunc(volatile bool& running, void* arg);
    bool getAndCompleteNextEntry();
};
