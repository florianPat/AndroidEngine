#pragma once

#include <jni.h>
#include "Utils.h"
#include "String.h"

struct jniUtils
{
    __thread static JNIEnv* jniEnv;
    static jobject activity;
    static JavaVM* vm;
    static struct ClassLoader
    {
        jobject object = nullptr;
        jmethodID findClassMethod = nullptr;
    } classLoader;
public:
    static jclass findClass(const String& clazz);
    static void checkException();
};

class JNIString
{
    jstring jString = nullptr;
    JNIEnv* jniEnv = nullptr;
    const char* cString = nullptr;
public:
    JNIString(jstring stringIn) : jString(stringIn),
                                  jniEnv(jniUtils::jniEnv),
                                  cString(jniEnv->GetStringUTFChars(jString, nullptr))
    {}
    JNIString(jstring stringIn, JNIEnv* jniEnv) : jString(stringIn),
                                                  jniEnv(jniEnv),
                                                  cString(jniEnv->GetStringUTFChars(jString, nullptr))
    {}

    static jstring create(const char* cString)
    {
        return jniUtils::jniEnv->NewStringUTF(cString);
    }
    static jstring create(const String& str)
    {
        return jniUtils::jniEnv->NewStringUTF(str.c_str());
    }

    ~JNIString()
    {
        jniEnv->ReleaseStringUTFChars(jString, cString);
    }

    const char* getCString() const
    {
        return cString;
    }
};