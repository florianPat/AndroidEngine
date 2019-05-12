#include "GraphicsOGL2.h"
#include "GLUtils.h"

GraphicsOGL2::GraphicsOGL2(int renderWidth, int renderHeight, IGraphics::ViewportType viewportType)
        : GraphicsOGLIniter(renderWidth, renderHeight, viewportType, DISPLAY_ATTRIBS),
          view(renderWidth, renderHeight), orhtoProj(view.getOrthoProj()),
          vertices(std::make_unique<Vertex[]>(NUM_VERTICES_TO_BATCH))
{
    Vector2f spriteVertices[] = { {0.0f, 0.0f},
                                  {1.0f, 0.0f},
                                  {1.0f, 1.0f},
                                  {0.0f, 1.0f} };
    for(int i = 0; i < NUM_VERTICES_TO_BATCH; i += 4)
    {
        vertices[i + 0].position = spriteVertices[0];
        vertices[i + 1].position = spriteVertices[1];
        vertices[i + 2].position = spriteVertices[2];
        vertices[i + 3].position = spriteVertices[3];
    }
}

void GraphicsOGL2::clear()
{
    CallGL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
    CallGL(glClear(GL_COLOR_BUFFER_BIT));
}

void GraphicsOGL2::draw(const Sprite& sprite)
{
    assert(vb);

    const Texture* texture = sprite.getTexture();
    assert(*texture);

    if(currentBoundTexture != texture->getTextureId())
    {
        if(nSpritesBatched != 0)
            flush();

        texture->bind();
        currentBoundTexture = texture->getTextureId();
    }
    else if(nSpritesBatched >= NUM_SPRITES_TO_BATCH)
    {
        flush();
    }

    float texRectLeft = ((float)sprite.getTextureRect().left) / texture->getWidth();
    float texRectTop = ((float)sprite.getTextureRect().top) / texture->getHeight();
    float texRectRight = ((float)sprite.getTextureRect().getRight()) / texture->getWidth();
    float texRectBottom = ((float)sprite.getTextureRect().getBottom()) / texture->getHeight();
    Vector2f texCoord[4] = { { texRectLeft, texRectTop },
                             { texRectRight, texRectTop },
                             { texRectRight, texRectBottom },
                             { texRectLeft, texRectBottom } };

    float red = sprite.getColor().r / 255.0f;
    float green = sprite.getColor().g / 255.0f;
    float blue = sprite.getColor().b / 255.0f;
    float alpha = sprite.getColor().a / 255.0f;

    Mat4x4 mv = sprite.getTransform();

    int plusI = nVerticesBatched();

    for(int i = 0; i < 4; ++i)
    {
        //TODO: I could also pass the pos, scl, angle in directly and compute the mv in the vertexShader
        // (But I would pass in one more float; and it is not really that much faster (according to what I saw))

        //TODO: Can add the active texture unit this sprite belongs to and batch sprites with different
        // textures!
        vertices[i + plusI].tex = texCoord[i];
        vertices[i + plusI].colorR = red;
        vertices[i + plusI].colorG = green;
        vertices[i + plusI].colorB = blue;
        vertices[i + plusI].colorA = alpha;
        vertices[i + plusI].mvMatrix[0] = mv.matrix[0];
        vertices[i + plusI].mvMatrix[1] = mv.matrix[1];
        vertices[i + plusI].mvMatrix[2] = mv.matrix[4];
        vertices[i + plusI].mvMatrix[3] = mv.matrix[5];
        vertices[i + plusI].mvMatrix[4] = mv.matrix[12];
        vertices[i + plusI].mvMatrix[5] = mv.matrix[13];
    }

    ++nSpritesBatched;
}

void GraphicsOGL2::draw(const RectangleShape& rect)
{
    assert(vb);

    flush();

    float vertices[] = {0.0f, 0.0f,
                        1.0f, 0.0f,
                        1.0f, 1.0f,
                        0.0f, 1.0f };
    vb.subData(0, sizeof(vertices), vertices);

    VertexLayouts va;
    va.addAttribute(2, GL_FLOAT);
    va.set();

    Mat4x4 mvp = orhtoProj * rect.getTransform();

    Color c = rect.getFillColor();

    shaderRectShape.bind();
    shaderRectShape.setUniformMat4f("u_mvp", mvp);
    shaderRectShape.setUniform4f("u_color", c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);

    CallGL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, 0));

    setupSpriteRendering();
}

void GraphicsOGL2::render()
{
    flush();

    EGLBoolean result = eglSwapBuffers(display, surface);
    assert(result);

    if (view.updated())
        orhtoProj = view.getOrthoProj();
}

void GraphicsOGL2::setupGfxGpu()
{
    view = View(renderWidth, renderHeight);
    orhtoProj = view.getOrthoProj();

    CallGL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &nTextureUnits));

    new (&shaderSprite) Shader("SSprite", Vector<ShortString>{ "position", "texCoord", "color",
                                                               "mvMatrixSclRot", "mvMatrixPos" });
    new (&shaderRectShape) Shader("SRectShape", Vector<ShortString>{ "position" });

    unsigned int indices[6 * NUM_SPRITES_TO_BATCH];
    for(uint i = 0, counter = 0; i < NUM_SPRITES_TO_BATCH; ++i, counter += 6)
    {
        indices[counter + 0] = 0 + i * 4;
        indices[counter + 1] = 2 + i * 4;
        indices[counter + 2] = 3 + i * 4;
        indices[counter + 3] = 0 + i * 4;
        indices[counter + 4] = 1 + i * 4;
        indices[counter + 5] = 2 + i * 4;
    }
    new (&ib) IndexBuffer(indices, arrayCount(indices));
    ib.bind();

    new (&vb) VertexBuffer(nullptr, sizeof(Vertex) * NUM_VERTICES_TO_BATCH, GL_DYNAMIC_DRAW);
    vb.bind();

    setupSpriteRendering();
}

void GraphicsOGL2::bindOtherOrthoProj(const Mat4x4& otherOrthoProj)
{
    assert(shaderSprite);
    shaderSprite.setUniformMat4f("u_proj", otherOrthoProj);
}

void GraphicsOGL2::unbindOtherOrthoProj()
{
    assert(shaderSprite);
    shaderSprite.setUniformMat4f("u_proj", orhtoProj);
}

void GraphicsOGL2::setupSpriteRendering()
{
    assert(shaderSprite);

    shaderSprite.bind();
    VertexLayouts va;
    va.addAttribute(2, GL_FLOAT);
    va.addAttribute(2, GL_FLOAT);
    va.addAttribute(4, GL_FLOAT);
    va.addAttribute(4, GL_FLOAT);
    va.addAttribute(2, GL_FLOAT);
    va.set();
}

void GraphicsOGL2::flush()
{
    assert(ib);
    assert(shaderSprite);

    vb.subData(0, sizeof(Vertex) * nVerticesBatched(), vertices.get());

    CallGL(glDrawElements(GL_TRIANGLES, 6 * nSpritesBatched, GL_UNSIGNED_INT, 0));
    nSpritesBatched = 0;
}

int GraphicsOGL2::nVerticesBatched() const
{
    return (nSpritesBatched * 4);
}

//TODO: Implement
void GraphicsOGL2::draw(const CircleShape& circle)
{
}
