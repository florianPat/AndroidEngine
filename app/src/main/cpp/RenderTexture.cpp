#include "RenderTexture.h"
#include "GLUtils.h"
#include "Utils.h"
#include "VertexBuffer.h"
#include "VertexLayout.h"
#include "IndexBuffer.h"

RenderTexture::RenderTexture(RenderTexture && other) : renderTexture(std::exchange(other.renderTexture, 0)), screenTexture(other.screenTexture),
													   texture(std::move(other.texture)), orhtoProj(other.orhtoProj)
{
}

RenderTexture & RenderTexture::operator=(RenderTexture && rhs)
{
	this->~RenderTexture();

	renderTexture = std::exchange(rhs.renderTexture, 0);
	screenTexture = rhs.screenTexture;
	texture = std::move(rhs.texture);
	orhtoProj = rhs.orhtoProj;

	return *this;
}

RenderTexture::~RenderTexture()
{
	CallGL(glDeleteFramebuffers(1, &renderTexture));
	screenTexture = 0;
}

bool RenderTexture::create(uint32_t width, uint32_t height)
{
	CallGL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*) &screenTexture));

	CallGL(glGenFramebuffers(1, &renderTexture));
	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, renderTexture));

	assert(!texture);
	new (&texture) Texture(nullptr, width, height);

	CallGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getTextureId(), 0));

	GLenum result;
	CallGL(result = glCheckFramebufferStatus(GL_FRAMEBUFFER));
	assert(result == GL_FRAMEBUFFER_COMPLETE);

	CallGL(glBindTexture(GL_TEXTURE_2D, 0));

	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, screenTexture));

	orhtoProj = Mat4x4::orthoProj(-1.0f, 1.0f, 0.0f, 0.0f, width, height);

	return true;
}

const Texture & RenderTexture::getTexture() const
{
	return texture;
}

RenderTexture::operator bool() const
{
	return (screenTexture != 0);
}

void RenderTexture::begin(Graphics& gfx)
{
	gfx.flush();

    CallGL(glBindFramebuffer(GL_FRAMEBUFFER, renderTexture));

    CallGL(glViewport(0, 0, texture.getWidth(), texture.getHeight()));

    gfx.bindOtherOrthoProj(orhtoProj);
}

void RenderTexture::end(Graphics& gfx)
{
	gfx.flush();

    gfx.unbindOtherOrthoProj();

    CallGL(glBindFramebuffer(GL_FRAMEBUFFER, screenTexture));

    CallGL(glViewport(0, 0, gfx.getDefaultView().getViewportSize().x, gfx.getDefaultView().getViewportSize().y));
}
