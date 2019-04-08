#pragma once

#include <GLES2/gl2.h>

class VertexBuffer
{
	GLuint rendererId;
public:
	VertexBuffer(const void* data, int size);
	VertexBuffer(const VertexBuffer& other) = delete;
	VertexBuffer(VertexBuffer&& other);
	VertexBuffer& operator=(const VertexBuffer& rhs) = delete;
	VertexBuffer& operator=(VertexBuffer& rhs);
	explicit operator bool() const;
	~VertexBuffer();
	void bind() const;
	void unbind() const;
};