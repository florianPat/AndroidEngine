#include "DirWalker.h"

AAssetManager* DirWalker::aassetManager = nullptr;

DirIterator::DirIterator(AAssetDir* assetDir) : assetDir(assetDir)
{
    operator++();
}

DirIterator& DirIterator::operator++()
{
    if(assetDir != nullptr)
    {
        dirname.clear();
        const char* nextDirname = AAssetDir_getNextFileName(assetDir);
        if(nextDirname != nullptr)
        {
            dirname.append(nextDirname);
        }
        else
        {
            assetDir = nullptr;
        }
    }

    return *this;
}

DirIterator::operator bool() const
{
    return (assetDir != nullptr);
}

const String& DirIterator::operator*() const
{
    return dirname;
}

DirWalker::DirWalker(const String& dirname)
{
    open(dirname);
}

DirWalker::DirWalker(DirWalker&& other) : assetDir(std::exchange(other.assetDir, nullptr)), good(std::exchange(other.good, false))
{
}

DirWalker& DirWalker::operator=(DirWalker&& rhs)
{
    this->~DirWalker();

    new (this) DirWalker(std::move(rhs));

    return *this;
}

DirWalker::~DirWalker()
{
    close();
}

DirWalker::operator bool() const
{
    return good;
}

bool DirWalker::operator!() const
{
    return !(operator bool());
}

void DirWalker::open(const String& filename)
{
    assetDir = AAssetManager_openDir(aassetManager, filename.c_str());

    if(assetDir)
    {
        good = true;
    }
}

void DirWalker::close()
{
    if(good)
    {
        AAssetDir_close(assetDir);
        good = false;
    }
}

void DirWalker::setAassetManager(AAssetManager* aassetManagerIn)
{
    aassetManager = aassetManagerIn;
}

DirIterator DirWalker::begin()
{
    AAssetDir_rewind(assetDir);
    return DirIterator(assetDir);
}
