#pragma once

#include <GLES2/gl2.h>

class IndexBuffer
{
	GLuint rendererId;
	int count;
public:
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