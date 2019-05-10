#pragma once

#include <GLES2/gl2.h>

class VertexBuffer
{
	//NOTE: Same as in Texture
	GLuint rendererId = -1;
public:
	VertexBuffer() = default;
	VertexBuffer(const void* data, int size, GLenum usage = GL_STATIC_DRAW);
	VertexBuffer(const VertexBuffer& other) = delete;
	VertexBuffer(VertexBuffer&& other);
	VertexBuffer& operator=(const VertexBuffer& rhs) = delete;
	VertexBuffer& operator=(VertexBuffer& rhs);
	explicit operator bool() const;
	~VertexBuffer();
	void bind() const;
	void unbind() const;
	void subData(int offset, int size, const void* data);
};