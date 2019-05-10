#pragma once

#include <GLES2/gl2.h>

class IndexBuffer
{
	//NOTE: Same as in Texture!
	GLuint rendererId = -1;
	int count = 0;
public:
	IndexBuffer() = default;
	IndexBuffer(unsigned int* indices, int count);
	IndexBuffer(const IndexBuffer& other) = delete;
	IndexBuffer(IndexBuffer&& other);
	IndexBuffer& operator=(const IndexBuffer& rhs) = delete;
	IndexBuffer& operator=(IndexBuffer&& rhs);
	~IndexBuffer();
	void bind() const;
	void unbind() const;
	int getCount() const;
	explicit operator bool() const;
};