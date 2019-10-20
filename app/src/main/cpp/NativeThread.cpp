#include "NativeThread.h"
#include "Utils.h"
#include <syscall.h>
#include <semaphore.h>

NativeThread::NativeThread(Delegate<void(volatile bool&, void*)> threadFunc, void* arg) : threadParameter{ threadFunc, jniUtils::vm, arg }
{
    start();
}

NativeThread::~NativeThread()
{
    stop();
}

static void* startRoutine(void* args)
{
    NativeThread::ThreadParameter* threadParameter = (NativeThread::ThreadParameter*)args;
    JavaVMAttachArgs javaVmAttachArgs = { 0 };
	javaVmAttachArgs.version = JNI_VERSION_1_6;
	javaVmAttachArgs.name = "NativeThread";
	javaVmAttachArgs.group = nullptr;

	//TODO: Fix that this jniEnv needs to get used!!
	JNIEnv* jniEnv;
	jint result = threadParameter->javaVm->AttachCurrentThreadAsDaemon(&jniEnv, &javaVmAttachArgs);
	assert(result == JNI_OK);

	threadParameter->threadFunc(threadParameter->running, threadParameter->arg);

	threadParameter->javaVm->DetachCurrentThread();
	utils::log("pthread_exit");
	pthread_exit(nullptr);
}

void NativeThread::start()
{
    utils::log("start thread");

    assert(thread == 0);

    pthread_attr_t attributes;
    int32_t result = pthread_attr_init(&attributes);
    assert(result == 0);
    result = pthread_create(&thread, &attributes, startRoutine, &threadParameter);
    assert(result == 0);
    result = pthread_attr_destroy(&attributes);
    assert(result == 0);

    setThreadAffinityMask(thread, ThreadAffinityMask::LITTLE);
}

void NativeThread::stop()
{
    assert(thread != 0);

    //TODO: Think about waiting for the thread to finish?
    threadParameter.running = false;
    pthread_detach(thread);
    thread = 0;
}

void NativeThread::setThreadAffinityMask(int32_t tid, NativeThread::ThreadAffinityMask mask)
{
    int32_t syscallResult = syscall(__NR_sched_setaffinity, tid, sizeof(mask), mask);
    assert(syscallResult);
}
