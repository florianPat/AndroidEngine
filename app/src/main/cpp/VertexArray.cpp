#if 0
#include "VertexArray.h"

#include "Utils.h"
#include "GLUtils.h"

int32_t VertexArray::sizeofType(GLenum type) const
{
	switch (type)
	{
		case GL_FLOAT:
		{
			return sizeof(GLfloat);
		}
		default:
		{
			InvalidCodePath;
			return 0;
		}
	}
}

VertexArray::VertexArray() : vertexLayouts()
{
	CallGL(glGenVertexArrays(1, &rendererId));
	CallGL(glBindVertexArray(rendererId));
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &rendererId);
}

void VertexArray::setVertexBuffer(const VertexBuffer * vb)
{
	this->vb = vb;
}

void VertexArray::setIndexBuffer(const IndexBuffer * ib)
{
	this->ib = ib;
}

void VertexArray::addAttribute(GLint size, GLenum type)
{
	VertexLayout vl = {};
	vl.size = size;
	vl.type = type;
	vl.normalized = false;

	stride += size * sizeofType(type);

	vertexLayouts.push_back(vl);
}

void VertexArray::bind() const
{
	assert(vb);
	assert(ib);
	assert(vertexLayouts.size() > 0);

	CallGL(glBindVertexArray(rendererId));
	vb->bind();

	int32_t offset = 0;

	for (uint32_t i = 0; i < vertexLayouts.size(); ++i)
	{
		auto& it = vertexLayouts[i];

		CallGL(glEnableVertexAttribArray(i));
		CallGL(glVertexAttribPointer(i, it.size, it.type, it.normalized, stride, (void*)offset));

		offset += it.size * sizeofType(it.type);
	}

	ib->bind();
}

int32_t VertexArray::getIboCount() const
{
	return ib->getCount();
}

#endif