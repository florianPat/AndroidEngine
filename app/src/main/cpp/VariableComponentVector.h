#pragma once

#include "Types.h"
#include "Component.h"

class VariableComponentVector
{
    uint32_t size;
    uint8_t* vector;
    uint32_t offsetToEnd = 0;
public:
    VariableComponentVector(uint32_t size);
    VariableComponentVector(const VariableComponentVector& other) = delete;
    VariableComponentVector& operator=(const VariableComponentVector& rhs) = delete;
    VariableComponentVector(VariableComponentVector&& other);
    VariableComponentVector& operator=(VariableComponentVector&& rhs);
    ~VariableComponentVector();

    inline uint8_t* begin() { return vector; }
    inline uint8_t* end() { return begin() + getOffsetToEnd(); }
    inline const uint8_t* begin() const { return vector; }
    inline const uint8_t* end() const { return begin() + getOffsetToEnd(); }

    inline void clear() { offsetToEnd = 0; }

    inline uint32_t getOffsetToEnd() const { return offsetToEnd; }

    template <typename T, typename... Args>
    void push_back(Args&&... args);
};

template <typename T, typename... Args>
inline void VariableComponentVector::push_back(Args&&... args)
{
    uint32_t newOffsetToEnd = offsetToEnd + sizeof(uint) + sizeof(T);
    assert(newOffsetToEnd <= size);

    uint32_t* pSize = (uint32_t*) &vector[offsetToEnd];
    *pSize = sizeof(T);
    offsetToEnd += sizeof(uint32_t);

    T* pPtr = (T*) &vector[offsetToEnd];
    new (pPtr) T(std::forward<Args>(args)...);

    offsetToEnd = newOffsetToEnd;
}
