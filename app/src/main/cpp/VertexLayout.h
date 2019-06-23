#pragma once

#include "Vector.h"
#include <GLES2/gl2.h>

class VertexLayouts
{
	struct VertexLayout
	{
		GLint size;
		GLenum type;
		GLboolean normalized;
	};
	GLsizei stride = 0;

	Vector<VertexLayout> vertexLayouts;
private:
	int32_t sizeofType(GLenum type) const;
public:
	VertexLayouts() = default;
	void addAttribute(GLint size, GLenum type);
	void set() const;
};