#include "IndexBuffer.h"
#include "GLUtils.h"

IndexBuffer::IndexBuffer(unsigned int * indices, int count) : count(count)
{
	CallGL(glGenBuffers(1, &rendererId));
	CallGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId));
	CallGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW));
}

IndexBuffer::IndexBuffer(IndexBuffer && other) : rendererId(std::exchange(other.rendererId, 0)), count(std::exchange(other.count, 0))
{
}

IndexBuffer & IndexBuffer::operator=(IndexBuffer && rhs)
{
	this->~IndexBuffer();

	rendererId = std::exchange(rhs.rendererId, 0);
	count = std::exchange(rhs.count, 0);

	return *this;
}

IndexBuffer::~IndexBuffer()
{
	CallGL(glDeleteBuffers(1, &rendererId));
	count = 0;
}

void IndexBuffer::bind() const
{
	CallGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId));
}

void IndexBuffer::unbind() const
{
	CallGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

int IndexBuffer::getCount() const
{
	return count;
}

IndexBuffer::operator bool() const
{
	return (count != 0);
}
