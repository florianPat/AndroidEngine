#pragma once
#if 0
#include <GLES3/gl3.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Vector.h"

//NOTE: Does not work with OpenGL ES 2
//Now I will just if it out

class VertexArray
{
	struct VertexLayout
	{
		GLint size;
		GLenum type;
		GLboolean normalized;
	};
	GLsizei stride = 0;

	GLuint rendererId;
	const VertexBuffer* vb = nullptr;
	const IndexBuffer* ib = nullptr;
	Vector<VertexLayout> vertexLayouts;
private:
	int32_t sizeofType(GLenum type) const;
public:
	VertexArray();
	~VertexArray();
	void setVertexBuffer(const VertexBuffer* vb);
	void setIndexBuffer(const IndexBuffer* ib);
	void addAttribute(GLint32_t size, GLenum type);
	void bind() const;
	void unbind() const;
	int32_t getIboCount() const;
};
#endif