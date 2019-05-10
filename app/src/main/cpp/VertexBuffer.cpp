#include "VertexBuffer.h"
#include "GLUtils.h"

VertexBuffer::VertexBuffer(const void * data, int size, GLenum usage)
{
	CallGL(glGenBuffers(1, &rendererId));
	CallGL(glBindBuffer(GL_ARRAY_BUFFER, rendererId));
	CallGL(glBufferData(GL_ARRAY_BUFFER, size, data, usage));
}

VertexBuffer::VertexBuffer(VertexBuffer && other) : rendererId(std::exchange(other.rendererId, 0))
{
}

VertexBuffer & VertexBuffer::operator=(VertexBuffer & rhs)
{
	this->~VertexBuffer();

	rendererId = std::exchange(rhs.rendererId, 0);

	return *this;
}

VertexBuffer::operator bool() const
{
	return (rendererId != -1);
}

VertexBuffer::~VertexBuffer()
{
	CallGL(glDeleteBuffers(1, &rendererId));
	rendererId = -1;
}

void VertexBuffer::bind() const
{
	CallGL(glBindBuffer(GL_ARRAY_BUFFER, rendererId));
}

void VertexBuffer::unbind() const
{
	CallGL(glBindBuffer(GL_ARRAY_BUFFER, rendererId));
}

void VertexBuffer::subData(int offset, int size, const void* data)
{
	CallGL(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
}
