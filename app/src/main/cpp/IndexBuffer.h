#pragma once

#include <GLES2/gl2.h>
#include "Types.h"

class IndexBuffer
{
	//NOTE: Same as in Texture!
	GLuint rendererId = -1;
	int32_t count = 0;
public:
	IndexBuffer() = default;
	IndexBuffer(uint32_t* indices, int32_t count);
	IndexBuffer(const IndexBuffer& other) = delete;
	IndexBuffer(IndexBuffer&& other);
	IndexBuffer& operator=(const IndexBuffer& rhs) = delete;
	IndexBuffer& operator=(IndexBuffer&& rhs);
	~IndexBuffer();
	void bind() const;
	int32_t getCount() const;
	explicit operator bool() const;
};