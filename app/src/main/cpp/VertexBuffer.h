#pragma once

#include <GLES2/gl2.h>

class VertexBuffer
{
	//NOTE: Same as in Texture
	GLuint rendererId = -1;
public:
	VertexBuffer() = default;
	VertexBuffer(const void* data, int32_t size, GLenum usage = GL_STATIC_DRAW);
	VertexBuffer(const VertexBuffer& other) = delete;
	VertexBuffer(VertexBuffer&& other);
	VertexBuffer& operator=(const VertexBuffer& rhs) = delete;
	VertexBuffer& operator=(VertexBuffer& rhs);
	explicit operator bool() const;
	~VertexBuffer();
	void bind() const;
	void subData(int32_t offset, int32_t size, const void* data);
};