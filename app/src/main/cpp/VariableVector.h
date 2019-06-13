#pragma once

#include "Vector.h"

//NOTE: Only the things I need are implemented!
class VariableVector
{
    std::unique_ptr<uchar[]> vector = nullptr;
    size_t offsetToEnd = 0;
public:
    inline uchar* begin() { return vector.get(); }
    inline uchar* end() { return begin() + getOffsetToEnd(); }
    inline const uchar* begin() const { return vector.get(); }
    inline const uchar* end() const { return begin() + getOffsetToEnd(); }

    inline void clear()
    {
        offsetToEnd = 0;
    }

    inline size_t getOffsetToEnd() const { return offsetToEnd; }

    template <typename T, typename... Args>
    void push_back(Args&&... args);
};

template <typename T, typename... Args>
inline void VariableVector::push_back(Args&&... args)
{
    size_t newOffsetToEnd = offsetToEnd + sizeof(uint) + sizeof(T);

    std::unique_ptr<uchar[]> newVector = std::make_unique<uchar[]>(newOffsetToEnd);
    assert((newOffsetToEnd % sizeof(uint)) == 0);

    uint* oldData = (uint*) vector.get();
    uint* newData = (uint*) newVector.get();

    //TODO (NOTE): I can "move" all types like this, can`t I?
    for(size_t i = 0; i < (offsetToEnd / sizeof(uint)); ++i)
    {
        newData[i] = oldData[i];
    }
    vector = std::move(newVector);

    uchar* data = vector.get();

    int* pSize = (int*) &data[offsetToEnd];
    *pSize = sizeof(T);
    offsetToEnd += sizeof(uint);

    T* pPtr = (T*) &data[offsetToEnd];
    new (pPtr) T(std::forward<Args>(args)...);

    offsetToEnd = newOffsetToEnd;
}