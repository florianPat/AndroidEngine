#pragma once

#include "Array.h"
#include <new>

template <typename T>
struct HeapArray : public Array<T, 0>
{
	HeapArray(size_t size);
	HeapArray(size_t count, const T& value);
	HeapArray(size_t count, size_t plusCount, const T& value);
	HeapArray(const HeapArray& other);
	HeapArray(HeapArray&& other);
	HeapArray(const HeapArray& other, size_t sizeIn);
	HeapArray(HeapArray&& other, size_t sizeIn);
	HeapArray& operator=(const HeapArray& other);
	HeapArray& operator=(HeapArray&& other);
};

template <typename T>
inline HeapArray<T>::HeapArray(size_t size) : Array<T, 0>{ { .heapArray.p = (T*)malloc(sizeof(T) * size), .heapArray.capacity = size } }
{
}

template <typename T>
inline HeapArray<T>::HeapArray(size_t count, const T & value) : HeapArray(count)
{
	for (size_t i = 0; i < count; ++i)
		new (&this->arrayUnion.heapArray.p[i]) T(value);

	this->arraySize = count;
}

template<typename T>
inline HeapArray<T>::HeapArray(size_t count, size_t plusCount, const T & value) : HeapArray(count + plusCount)
{
	for (size_t i = 0; i < count; ++i)
		new (&this->arrayUnion.heapArray.p[i]) T(value);

	this->arraySize = count;
}

template <typename T>
inline HeapArray<T>::HeapArray(const HeapArray & other) : HeapArray(other.arrayUnion.heapArray.capacity)
{
	for (size_t i = 0; i < other.arraySize; ++i)
		new (&this->arrayUnion.heapArray.p[i]) T(other.arrayUnion.heapArray.p[i]);

	this->arraySize = other.arraySize;
}

template <typename T>
inline HeapArray<T>::HeapArray(HeapArray && other) : Array<T, 0>{ { .heapArray.p = std::exchange(other.arrayUnion.heapArray.p, nullptr), 
													 .heapArray.capacity = std::exchange(other.arrayUnion.heapArray.capacity, 0) }, std::exchange(other.arraySize, 0) }
{
}

template <typename T>
inline HeapArray<T>::HeapArray(const HeapArray & other, size_t capacityIn) : HeapArray(capacityIn)
{
	//NOTE: Why would you call this then?
	assert(other.arrayUnion.heapArray.capacity != capacityIn);

	size_t size;
	if (other.arraySize < capacityIn)
		size = other.arraySize;
	else
		size = capacityIn;

	for (size_t i = 0; i < size; ++i)
	{
		new (&this->arrayUnion.heapArray.p[i]) T(other.arrayUnion.heapArray.p[i]);
	}
	this->arraySize = size;
}

template <typename T>
inline HeapArray<T>::HeapArray(HeapArray && other, size_t capacityIn) : HeapArray(capacityIn)
{
	//NOTE: Why would you call this then?
	assert(other.arrayUnion.heapArray.capacity != capacityIn);

	size_t size = 0;
	if (other.arraySize <= capacityIn)
		size = other.arraySize;
	else if (other.arraySize > capacityIn)
	{
		for (size_t i = capacityIn; i < other.arraySize; ++i)
			other.arrayUnion.heapArray.p[i].~T();

		size = capacityIn;
	}
	else
		InvalidCodePath;

	for (size_t i = 0; i < size; ++i)
		new (&this->arrayUnion.heapArray.p[i]) T(std::move(other.arrayUnion.heapArray.p[i]));

	free(other.arrayUnion.heapArray.p);
	other.arrayUnion.heapArray.p = nullptr;
	other.arrayUnion.heapArray.capacity = 0;
	other.arraySize = 0;

	this->arraySize = size;
}

template <typename T>
inline HeapArray<T> & HeapArray<T>::operator=(const HeapArray<T> & other)
{
	this->~Array<T, 0>();

	new (this) HeapArray(other);

	return *this;
}

template <typename T>
inline HeapArray<T> & HeapArray<T>::operator=(HeapArray<T> && other)
{
	this->~Array<T, 0>();

	new (this) HeapArray(std::move(other));

	return *this;
}

