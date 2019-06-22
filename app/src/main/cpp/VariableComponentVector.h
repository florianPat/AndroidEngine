#pragma once

#include "Types.h"
#include "Component.h"

class VariableComponentVector
{
    size_t size;
    uchar* vector;
    size_t offsetToEnd = 0;
public:
    VariableComponentVector(size_t size);
    VariableComponentVector(const VariableComponentVector& other) = delete;
    VariableComponentVector& operator=(const VariableComponentVector& rhs) = delete;
    VariableComponentVector(VariableComponentVector&& other);
    VariableComponentVector& operator=(VariableComponentVector&& rhs);
    ~VariableComponentVector();

    inline uchar* begin() { return vector; }
    inline uchar* end() { return begin() + getOffsetToEnd(); }
    inline const uchar* begin() const { return vector; }
    inline const uchar* end() const { return begin() + getOffsetToEnd(); }

    inline void clear() { offsetToEnd = 0; }

    inline size_t getOffsetToEnd() const { return offsetToEnd; }

    template <typename T, typename... Args>
    void push_back(Args&&... args);
};

template <typename T, typename... Args>
inline void VariableComponentVector::push_back(Args&&... args)
{
    size_t newOffsetToEnd = offsetToEnd + sizeof(uint) + sizeof(T);
    assert(newOffsetToEnd <= size);

    uint* pSize = (uint*) &vector[offsetToEnd];
    *pSize = sizeof(T);
    offsetToEnd += sizeof(uint);

    T* pPtr = (T*) &vector[offsetToEnd];
    new (pPtr) T(std::forward<Args>(args)...);

    offsetToEnd = newOffsetToEnd;
}
