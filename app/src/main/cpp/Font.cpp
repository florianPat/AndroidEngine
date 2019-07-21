#include "Font.h"
#include "Ifstream.h"
#include "Globals.h"
#include FT_IMAGE_H

bool Font::createGlyphRenderTextureAndMap(FT_Face& face)
{
    assert(gfx != nullptr);

    //NOTE: -1 because ' ' does not get rendered!
    uint renderTextureSize = (uint) size * (NUM_GLYPHS - 1);

    int32_t maxTextureSize = 0;
    uint32_t renderTextureRowSize = size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    if(renderTextureSize > maxTextureSize)
    {
        uint32_t nRows = (uint32_t)ceil(((float)renderTextureSize) / maxTextureSize);
        assert((nRows * size) < maxTextureSize);
        renderTextureSize = (uint32_t)ceil(((float)renderTextureSize) / nRows);
        renderTextureRowSize = size * nRows;
    }

    renderTexture.create(renderTextureSize, renderTextureRowSize);
    renderTexture.begin(*gfx);
    gfx->clear();
    uint32_t* pixels = (uint32_t*) malloc(size * size * sizeof(uint));

    Vector2i xy = { 0, 0 };
    Texture texture;
    Sprite sprite;

    if(!loadGlyphIntoMap(' ', regions[' ' - ' '], face, xy))
        return false;

    for(char c = '!'; c <= '~'; ++c)
    {
        GlyphRegion& glyphRegion = regions[c - ' '];
        if(!loadGlyphIntoMap(c, glyphRegion, face, xy))
            return false;

        int error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if(error)
        {
            utils::log("could not render char from face");

            destructFace(face);
            return false;
        }

        uint16_t numGrayLevels = face->glyph->bitmap.num_grays;
        for(int y = 0, pixelY = glyphRegion.size.y - 1; y < glyphRegion.size.y; ++y, --pixelY)
        {
            for(int x = 0; x < glyphRegion.size.x; ++x)
            {
                uint8_t currentPixelGrayLevel = face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
                //NOTE: Premultiplied alpha for free ;)
                uint8_t color = (uint8_t) ((float) currentPixelGrayLevel / numGrayLevels * 255.0f);
                pixels[pixelY * face->glyph->bitmap.pitch + x] = (color | (color << 8u) |
                                                                  (color << 16u) | (color << 24u));
            }
        }

        texture = Texture(pixels, face->glyph->bitmap.pitch, glyphRegion.size.y);
        sprite.setTexture(&texture, true);
        sprite.pos = (Vector2f) xy;
        gfx->draw(sprite);
        gfx->flush();

        xy.x += texture.getWidth();
        if(xy.x >= renderTextureSize)
        {
            xy.x = 0;
            xy.y += size;
            assert(xy.y <= renderTextureRowSize);
        }
    }

    renderTexture.end(*gfx);
    free(pixels);
    destructFace(face);

    return true;
}

bool Font::reloadFromFile(const String& filename)
{
    faceHeight = 0;
    renderTexture.~RenderTexture();

    FT_Face face = nullptr;

    Ifstream file(filename);
    assert(file);

    if(!loadFaceFromLibrary(file.getFullData(), file.getSize(), face))
        return false;

    return createGlyphRenderTextureAndMap(face);
}

void Font::drawText(const String& text, const Vector2f& pos, int pixelSize, Color color)
{
    Vector2f pen = { 0.0f, 0.0f };
    float scale = (float)pixelSize / size;
    Sprite sprite;
    sprite.color = color;
    sprite.setScale(scale);

    for(int i = 0; i < text.size(); ++i)
    {
        char c = text[i];

        if(c == '\n')
        {
            pen.y -= faceHeight * scale;
            pen.x = 0;
        }
        else if(c == ' ')
        {
            const GlyphRegion& region = regions[c - ' '];
            pen.x += region.advanceX * scale;
        }
        else
        {
            assert(c >= '!' && c <= '~');
            const GlyphRegion& region = regions[c - ' '];
            sprite.setTexture(&renderTexture.getTexture());
            sprite.setTextureRect(IntRect(region.xy.x, region.xy.y, region.size.x,
                                          region.size.y));

            sprite.pos = pos + pen + region.bitmapTopLeft;

            pen.x += region.advanceX * scale;

            gfx->draw(sprite);
        }
    }
}

Font::Font(Font&& other) : size(std::exchange(other.size, 0)), faceHeight(std::exchange(other.faceHeight, 0)),
                           renderTexture(std::exchange(other.renderTexture, RenderTexture())),
                           library(std::exchange(other.library, nullptr)), gfx(std::exchange(other.gfx, nullptr))
{
    for(int i = 0; i < NUM_GLYPHS; ++i)
    {
        new (&regions[i]) GlyphRegion(std::exchange(other.regions[i], GlyphRegion()));
    }
}

Font& Font::operator=(Font&& rhs)
{
    this->~Font();

    faceHeight = std::exchange(rhs.faceHeight, 0);
    size = std::exchange(rhs.size, 0);
    renderTexture = std::exchange(rhs.renderTexture, RenderTexture());
    library = std::exchange(rhs.library, nullptr);
    gfx = std::exchange(rhs.gfx, nullptr);

    for(int i = 0; i < NUM_GLYPHS; ++i)
    {
        new (&regions[i]) GlyphRegion(std::exchange(rhs.regions[i], GlyphRegion()));
    }

    return *this;
}

bool Font::loadFaceFromLibrary(const void* fileBuffer, uint64_t fileSize, FT_Face& face)
{
    int error = FT_New_Memory_Face(library, (const uint8_t*) fileBuffer, fileSize, 0, &face);
    if(error)
    {
        utils::log("could not create face");
        destructFace(face);
        return false;
    }

    error = FT_Set_Pixel_Sizes(face, size, 0);
    if(error)
    {
        utils::log("could not set pixel size of face!");
        destructFace(face);
        return false;
    }

    faceHeight = (uint) face->size->metrics.height >> 6u;

    if(face->charmap == nullptr)
    {
        error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        if(error)
        {
            utils::log("could not load a charmap");

            error = FT_Select_Charmap(face, FT_ENCODING_OLD_LATIN_2);
            if(error)
            {
                utils::log("could not load fallback charmap");
                destructFace(face);
                return false;
            }
        }
    }
    assert(face->charmap->encoding == FT_ENCODING_UNICODE || face->charmap->encoding == FT_ENCODING_OLD_LATIN_2);

    return true;
}

Font::operator bool() const
{
    return (faceHeight != 0);
}

void Font::destructFace(FT_Face& face)
{
    FT_Done_Face(face);
}

bool Font::loadGlyphIntoMap(char c, GlyphRegion& glyphRegion, FT_Face& face, Vector2i& xy)
{
    uint glyphIndex = FT_Get_Char_Index(face, c);
    int error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    if(error)
    {
        utils::log("could not load char from face");

        destructFace(face);
        return false;
    }
    else
    {
        glyphRegion.xy = xy;
        glyphRegion.size = {(int) face->glyph->bitmap.width,
                            (int) face->glyph->bitmap.rows};
        glyphRegion.bitmapTopLeft.x = face->glyph->bitmap_left;
        glyphRegion.bitmapTopLeft.y = (face->glyph->bitmap_top - glyphRegion.size.y);
        glyphRegion.advanceX = ((uint) face->glyph->advance.x) >> 6u;

        assert(face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
        assert(glyphRegion.size.x <= size && glyphRegion.size.y <= size);
    }

    return true;
}

void Font::drawText(const String& text, const Vector2f& pos, Color color)
{
    drawText(text, pos, size, color);
}

Font::Font(const String& filename, int size) : size(size)
{
    library = Globals::window->getFontLibrary();
    gfx = &Globals::window->getGfx();
    assert(library != nullptr && size != 0 && gfx != nullptr);

    FT_Face face = nullptr;

    Ifstream file(filename);
    assert(file);

    if(!loadFaceFromLibrary(file.getFullData(), file.getSize(), face))
        InvalidCodePath;

    if(!createGlyphRenderTextureAndMap(face))
        InvalidCodePath;
}
