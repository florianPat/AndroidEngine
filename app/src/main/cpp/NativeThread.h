#pragma once

#include <pthread.h>
#include <unistd.h>
#include "Delegate.h"
#include "JNIUtils.h"
#include "EventData.h"

class NativeThread
{
public:
    struct ThreadParameter
    {
        Delegate<void(volatile bool&, void*)> threadFunc;
        JavaVM* javaVm = nullptr;
        void* arg;
        volatile bool running = true;
    };
private:
    enum class ThreadAffinityMask
    {
        LITTLE = 0x0F,
        BIG = 0xF0
    };
    static constexpr int32_t PAUSED_THREAD = -1;
private:
    ThreadParameter threadParameter;
    pthread_t thread = 0;
public:
    NativeThread(Delegate<void(volatile bool&, void*)> threadFunc, void* arg);
    ~NativeThread();
    NativeThread(const NativeThread& other) = delete;
    NativeThread& operator=(const NativeThread& rhs) = delete;
    NativeThread(NativeThread&& other) = delete;
    NativeThread& operator=(NativeThread&& rhs) = delete;

    void start();
    void stop();
private:
    void setThreadAffinityMask(int32_t tid, ThreadAffinityMask mask);
};
