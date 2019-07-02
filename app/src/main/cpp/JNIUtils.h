#pragma once

#include <jni.h>
#include "Utils.h"
#include "String.h"

struct jniUtils
{
    static JNIEnv* jniEnv;
    static jobject activity;
    static JavaVM* vm;
    static struct ClassLoader
    {
        jobject object = nullptr;
        jmethodID findClassMethod = nullptr;
    } classLoader;
public:
    static jclass findClass(const String& clazz);
};

class JNIString
{
    jstring jString;
    const char* cString = nullptr;
public:
    JNIString(jstring stringIn) : jString(stringIn),
                                  cString(jniUtils::jniEnv->GetStringUTFChars(jString, nullptr))
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
        jniUtils::jniEnv->ReleaseStringUTFChars(jString, cString);
    }

    const char* getCString() const
    {
        return cString;
    }
};