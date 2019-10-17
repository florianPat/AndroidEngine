#pragma once

#include "Types.h"
#include "Component.h"

template <typename T>
class VariableVector
{
    uint32_t size;
    uint8_t* vector;
    uint32_t offsetToEnd = 0;
public:
    VariableVector(uint32_t size);
    VariableVector(const VariableVector& other) = delete;
    VariableVector& operator=(const VariableVector& rhs) = delete;
    VariableVector(VariableVector&& other);
    VariableVector& operator=(VariableVector&& rhs);
    ~VariableVector();

    inline uint8_t* begin() { return vector; }
    inline uint8_t* end() { return begin() + getOffsetToEnd(); }
    inline const uint8_t* begin() const { return vector; }
    inline const uint8_t* end() const { return begin() + getOffsetToEnd(); }

    inline void clear() { offsetToEnd = 0; }

    inline uint32_t getOffsetToEnd() const { return offsetToEnd; }

    template <typename Class, typename... Args>
    void push_back(Args&&... args);
};

template <typename T>
template <typename Class, typename... Args>
inline void VariableVector<T>::push_back(Args&&... args)
{
    uint32_t newOffsetToEnd = offsetToEnd + sizeof(uint) + sizeof(Class);
    assert(newOffsetToEnd <= size);

    uint32_t* pSize = (uint32_t*) &vector[offsetToEnd];
    *pSize = sizeof(Class);
    offsetToEnd += sizeof(uint32_t);

    Class* pPtr = (Class*) &vector[offsetToEnd];
    offsetToEnd = newOffsetToEnd;
    new (pPtr) Class(std::forward<Args>(args)...);
    assert(dynamic_cast<T*>(pPtr) != nullptr);
}

template <typename T>
inline VariableVector<T>::~VariableVector()
{
    for (auto it = begin(); it != end();)
    {
        uint compSize = *((uint*) it);
        it += sizeof(uint);

        ((T*)(it))->~T();

        it += compSize;
    }

    free(vector);
    vector = nullptr;
}

template <typename T>
inline VariableVector<T>::VariableVector(VariableVector&& other)
        :   size(std::exchange(other.size, 0)), vector(std::exchange(other.vector, nullptr)), offsetToEnd(std::exchange(other.offsetToEnd, 0))
{
}

template <typename T>
inline VariableVector<T>::VariableVector(uint32_t size) : size(size), vector((uint8_t*)malloc(size))
{
}

template <typename T>
inline VariableVector<T>& VariableVector<T>::operator=(VariableVector&& rhs)
{
    this->~VariableVector();

    size = std::exchange(rhs.size, 0);
    vector = std::exchange(rhs.vector, nullptr);
    offsetToEnd = std::exchange(rhs.offsetToEnd, 0);

    return *this;
}
