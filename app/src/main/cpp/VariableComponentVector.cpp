#include "VariableComponentVector.h"

VariableComponentVector::~VariableComponentVector()
{
    for (auto it = begin(); it != end();)
    {
        uint compSize = *((uint*) it);
        it += sizeof(uint);

        ((Component*)(it))->~Component();

        it += compSize;
    }

    free(vector);
    vector = nullptr;
}

VariableComponentVector::VariableComponentVector(VariableComponentVector&& other)
    :   size(std::exchange(other.size, 0)), vector(std::exchange(other.vector, nullptr)), offsetToEnd(std::exchange(other.offsetToEnd, 0))
{
}

VariableComponentVector::VariableComponentVector(size_t size) : size(size), vector((uchar*)malloc(size))
{
}

VariableComponentVector& VariableComponentVector::operator=(VariableComponentVector&& rhs)
{
    this->~VariableComponentVector();

    size = std::exchange(rhs.size, 0);
    vector = std::exchange(rhs.vector, nullptr);
    offsetToEnd = std::exchange(rhs.offsetToEnd, 0);

    return *this;
}
