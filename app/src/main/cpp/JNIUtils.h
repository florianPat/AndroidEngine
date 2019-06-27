#pragma once

#include <jni.h>
#include "Utils.h"

namespace jniUtils
{
    inline static JNIEnv* jniEnv = nullptr;
    inline static jobject activity = nullptr;
}

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