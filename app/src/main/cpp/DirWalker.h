#pragma once

#include "android_native_app_glue.h"
#include "String.h"

class DirIterator
{
    AAssetDir* assetDir = nullptr;
    LongString dirname;
public:
    DirIterator(AAssetDir* assetDir);
    DirIterator& operator++();
    explicit operator bool() const;
    const String& operator*() const;
};

class DirWalker
{
    AAssetDir* assetDir = nullptr;
    static AAssetManager* aassetManager;
    bool good = false;
public:
    DirWalker() = default;
    DirWalker(const String& dirname);
    DirWalker(const DirWalker& other) = delete;
    DirWalker(DirWalker&& other);
    DirWalker& operator=(const DirWalker& rhs) = delete;
    DirWalker& operator=(DirWalker&& rhs);
    ~DirWalker();
    explicit operator bool() const;
    bool operator!() const;
    void open(const String& filename);
    void close();
    DirIterator begin();
    static void setAassetManager(AAssetManager* aassetManager);
};
