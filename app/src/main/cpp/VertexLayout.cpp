#include "VertexLayout.h"
#include "Utils.h"
#include "GLUtils.h"
#include <GLES2/gl2.h>

int32_t VertexLayouts::sizeofType(GLenum type) const
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

void VertexLayouts::addAttribute(GLint size, GLenum type)
{
	VertexLayout vl = {};
	vl.size = size;
	vl.type = type;
	vl.normalized = false;

	stride += size * sizeofType(type);

	vertexLayouts.push_back(vl);
}

void VertexLayouts::set() const
{
	assert(vertexLayouts.size() > 0);

	int32_t offset = 0;

	for (uint32_t i = 0; i < vertexLayouts.size(); ++i)
	{
		auto& it = vertexLayouts[i];

		CallGL(glEnableVertexAttribArray(i));
		CallGL(glVertexAttribPointer(i, it.size, it.type, it.normalized, stride, (void*)offset));

		offset += it.size * sizeofType(it.type);
	}
}
