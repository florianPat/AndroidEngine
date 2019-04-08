#include "RenderTexture.h"
#include "GLUtils.h"
#include "Utils.h"
#include "VertexBuffer.h"
#include "VertexLayout.h"
#include "IndexBuffer.h"

RenderTexture::RenderTexture(RenderTexture && other) : renderTexture(std::exchange(other.renderTexture, 0)), screenTexture(other.screenTexture),
													   texture(std::move(other.texture)), orhtoProj(other.orhtoProj), 
													   shaderSprite(std::exchange(other.shaderSprite, nullptr)), windowWidth(other.windowWidth),
													   windowHeight(other.windowHeight)
{
}

RenderTexture & RenderTexture::operator=(RenderTexture && rhs)
{
	this->~RenderTexture();

	renderTexture = std::exchange(rhs.renderTexture, 0);
	screenTexture = rhs.screenTexture;
	texture = std::move(rhs.texture);
	orhtoProj = rhs.orhtoProj;
	shaderSprite = std::exchange(rhs.shaderSprite, nullptr);
	windowWidth = rhs.windowWidth;
	windowHeight = rhs.windowHeight;

	return *this;
}

RenderTexture::~RenderTexture()
{
	CallGL(glDeleteFramebuffers(1, &renderTexture));
}

bool RenderTexture::create(uint width, uint height, Shader* shaderSpriteIn)
{
	shaderSprite = shaderSpriteIn;
	GLint viewportSize[4] = { 0 };
	CallGL(glGetIntegerv(GL_VIEWPORT, viewportSize));
	windowWidth = viewportSize[2];
	windowHeight = viewportSize[3];

	CallGL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &screenTexture));

	CallGL(glGenFramebuffers(1, &renderTexture));
	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, renderTexture));

	GLuint textureId = 0;

	CallGL(glGenTextures(1, &textureId));

	texture = Texture(textureId, width, height);

	CallGL(glActiveTexture(GL_TEXTURE0));
	CallGL(glBindTexture(GL_TEXTURE_2D, textureId));

	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	CallGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	CallGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

	CallGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));

	GLenum result;
	CallGL(result = glCheckFramebufferStatus(GL_FRAMEBUFFER));
	assert(result == GL_FRAMEBUFFER_COMPLETE);

	CallGL(glBindTexture(GL_TEXTURE_2D, 0));

	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, screenTexture));

	orhtoProj = Mat4x4::orthoProj(-1.0f, 1.0f, 0.0f, 0.0f, width, height);

	return true;
}

void RenderTexture::clear()
{
	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, renderTexture));

	CallGL(glViewport(0, 0, texture.getWidth(), texture.getHeight()));

	CallGL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	CallGL(glClear(GL_COLOR_BUFFER_BIT));

	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, screenTexture));

	CallGL(glViewport(0, 0, windowWidth, windowHeight));
}

const Texture & RenderTexture::getTexture() const
{
	return texture;
}

void RenderTexture::display()
{
}

void RenderTexture::draw(const Sprite & sprite)
{
	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, renderTexture));

	CallGL(glViewport(0, 0, texture.getWidth(), texture.getHeight()));

	const Texture* texture = sprite.getTexture();
	assert(*texture);

	texture->bind();

	float texRectLeft = sprite.getTextureRect().left / texture->getWidth();
	float texRectTop = sprite.getTextureRect().top / texture->getHeight();
	float texRectRight = sprite.getTextureRect().getRight() / texture->getWidth();
	float texRectBottom = sprite.getTextureRect().getBottom() / texture->getHeight();

	Vector2f texCoord[4] = { { texRectLeft, texRectTop },
	{ texRectRight, texRectTop },
	{ texRectRight, texRectBottom },
	{ texRectLeft, texRectBottom } };

	float vertices[] = { 0.0f, 0.0f, texCoord[0].x, texCoord[0].y,
		1.0f, 0.0f, texCoord[1].x, texCoord[1].y,
		1.0f, 1.0f, texCoord[2].x, texCoord[2].y,
		0.0f, 1.0f, texCoord[3].x, texCoord[3].y };

	unsigned int indices[] = { 0, 2, 3, 0, 1, 2 };

	VertexBuffer vb = VertexBuffer(vertices, sizeof(vertices));
	IndexBuffer ib = IndexBuffer(indices, arrayCount(indices));

	vb.bind();

	VertexLayouts va;
	va.addAttribute(2, GL_FLOAT);
	va.addAttribute(2, GL_FLOAT);
	va.set();

	ib.bind();

	Mat4x4 mvp = orhtoProj * sprite.getTransform();

	Color c = sprite.getColor();

	shaderSprite->bind();
	shaderSprite->setUniformMat4f("u_mvp", mvp);
	shaderSprite->setUniform4f("u_color", c.r / 255, c.g / 255, c.b / 255, c.a / 255);

	CallGL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, 0));

	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, screenTexture));

	CallGL(glViewport(0, 0, windowWidth, windowHeight));
}
