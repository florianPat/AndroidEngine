#include "OGLRenderTexture.h"
#include "GLUtils.h"
#include "Utils.h"
#include "VertexBuffer.h"
#include "VertexLayout.h"
#include "IndexBuffer.h"

OGLRenderTexture::OGLRenderTexture(OGLRenderTexture && other) : renderTexture(std::exchange(other.renderTexture, 0)), screenTexture(other.screenTexture),
													   texture(std::move(other.texture)), orhtoProj(other.orhtoProj)
{
}

OGLRenderTexture & OGLRenderTexture::operator=(OGLRenderTexture && rhs)
{
	this->~OGLRenderTexture();

	renderTexture = std::exchange(rhs.renderTexture, 0);
	screenTexture = rhs.screenTexture;
	texture = std::move(rhs.texture);
	orhtoProj = rhs.orhtoProj;

	return *this;
}

OGLRenderTexture::~OGLRenderTexture()
{
	CallGL(glDeleteFramebuffers(1, &renderTexture));
	screenTexture = 0;
}

void OGLRenderTexture::create(uint32_t width, uint32_t height, GraphicsOGL2* gfxIn)
{
    gfx = gfxIn;

	CallGL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*) &screenTexture));

	CallGL(glGenFramebuffers(1, &renderTexture));
	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, renderTexture));

	assert(!texture);
	new (&texture) OGLTexture(nullptr, width, height);

	CallGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getTextureId(), 0));

	GLenum result;
	CallGL(result = glCheckFramebufferStatus(GL_FRAMEBUFFER));
	assert(result == GL_FRAMEBUFFER_COMPLETE);

	CallGL(glBindTexture(GL_TEXTURE_2D, 0));

	CallGL(glBindFramebuffer(GL_FRAMEBUFFER, screenTexture));

	orhtoProj = Mat4x4::orthoProjOGL(0.0f, 0.0f, width, height);
}

const OGLTexture & OGLRenderTexture::getTexture() const
{
	return texture;
}

OGLRenderTexture::operator bool() const
{
	return (renderTexture != (uint32_t)-1);
}

void OGLRenderTexture::begin()
{
	gfx->flush();

    CallGL(glBindFramebuffer(GL_FRAMEBUFFER, renderTexture));

    CallGL(glViewport(0, 0, texture.getWidth(), texture.getHeight()));

    gfx->bindOtherOrthoProj(orhtoProj);
}

void OGLRenderTexture::end()
{
	gfx->flush();

    gfx->unbindOtherOrthoProj();

    CallGL(glBindFramebuffer(GL_FRAMEBUFFER, screenTexture));

    CallGL(glViewport(0, 0, gfx->getDefaultView().getViewportSize().x, gfx->getDefaultView().getViewportSize().y));
}