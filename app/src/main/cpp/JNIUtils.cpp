#include "JNIUtils.h"

JNIEnv* jniUtils::jniEnv = nullptr;
jobject jniUtils::activity = nullptr;
JavaVM* jniUtils::vm = nullptr;
jniUtils::ClassLoader jniUtils::classLoader = { 0 };

jclass jniUtils::findClass(const String& clazz)
{
    return ((jclass) jniUtils::jniEnv->CallObjectMethod(jniUtils::classLoader.object,
            jniUtils::classLoader.findClassMethod, JNIString::create(clazz)));
}
