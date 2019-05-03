#include "Font.h"
#include "Ifstream.h"
#include "RenderWindow.h"
#include FT_IMAGE_H

bool Font::loadFromFile(const String& filename, void* fontOpts)
{
    FontOptions* opts = (FontOptions*) fontOpts;
    library = opts->renderWindow.getFontLibrary();
    spriteShader = opts->renderWindow.getSpriteShader();
    size = opts->size;
    assert(library != nullptr && opts->size != 0 && spriteShader != nullptr);

    FT_Face face = nullptr;

    Ifstream file(filename);
    assert(file);

    if(!loadFaceFromLibrary(file.getFullData(), file.getSize(), face))
        return false;

    return createGlyphRenderTextureAndMap(face);
}

bool Font::createGlyphRenderTextureAndMap(FT_Face& face)
{
    assert(spriteShader != nullptr);

    //NOTE: -1 because ' ' does not get rendered!
    uint renderTextureSize = (uint) size * (NUM_GLYPHS - 1);
    renderTexture.create(renderTextureSize, size, spriteShader);
    renderTexture.clear(Color(0, 0, 0, 0));
    uint* pixels = (uint*) malloc(size * size * sizeof(uint));

    Vector2i xy = { 0, 0 };
    for(uchar c = ' '; c <= '~'; ++c)
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
            error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            if(error)
            {
                utils::log("could not render char from face");

                destructFace(face);
                return false;
            }
            else
            {
                GlyphRegion& glyphRegion = regions[c - ' '];
                glyphRegion.xy = xy;
                glyphRegion.size = { (int) face->glyph->bitmap.width,
                                     (int) face->glyph->bitmap.rows };
                glyphRegion.bitmapTopLeft.x = face->glyph->bitmap_left;
                glyphRegion.bitmapTopLeft.y = (face->glyph->bitmap_top - glyphRegion.size.y);
                glyphRegion.advanceX = ((uint) face->glyph->advance.x) >> 6u;

                assert(face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
                assert(glyphRegion.size.x <= size && glyphRegion.size.y <= size);

                ushort numGrayLevels = face->glyph->bitmap.num_grays;
                for(int y = 0, pixelY = glyphRegion.size.y - 1; y < glyphRegion.size.y; ++y, --pixelY)
                {
                    for(int x = 0; x < glyphRegion.size.x; ++x)
                    {
                        uchar currentPixelGrayLevel = face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
                        /*currentPixelGrayLevel = currentPixelGrayLevel >= (uchar)128 ?
                                                (uchar)128 - (currentPixelGrayLevel - (uchar)128) :
                                                (uchar)128 + ((uchar)128 - currentPixelGrayLevel);*/
                        uchar color = (uchar) ((float) currentPixelGrayLevel / numGrayLevels * 255.0f);
                        pixels[pixelY * face->glyph->bitmap.pitch + x] = (color | (color << 8u) |
                                                                          (color << 16u) | (color << 24u));
                    }
                }

                //TODO: Can I get rid of this branch?
                if(c != ' ')
                {
                    texture = Texture(pixels, glyphRegion.size.x, glyphRegion.size.y);
                    Sprite sprite(&texture);
                    sprite.setPosition((Vector2f) xy);
                    renderTexture.draw(sprite);

                    xy.x += texture.getWidth();
                    if((xy.x + size) >= renderTextureSize)
                        InvalidCodePath;
                }
            }
        }
    }

    renderTexture.display();

    free(pixels);
    destructFace(face);

    return true;
}

bool Font::reloadFromFile(const String& filename)
{
    this->~Font();

    FT_Face face = nullptr;

    Ifstream file(filename);
    assert(file);

    if(!loadFaceFromLibrary(file.getFullData(), file.getSize(), face))
        return false;

    return createGlyphRenderTextureAndMap(face);
}

void Font::drawText(const String& text, const Vector2f& pos, RenderWindow& renderWindow)
{
    Vector2f pen = { 0.0f, 0.0f };

    for(int i = 0; i < text.size(); ++i)
    {
        char c = text[i];

        if(c == '\n')
        {
            pen.y -= faceHeight;
            pen.x = 0;
        }
        else if(c == ' ')
        {
            const GlyphRegion& region = regions[c - ' '];
            pen.x += region.advanceX;
        }
        else
        {
            assert(c >= '!' && c <= '~');
            const GlyphRegion& region = regions[c - ' '];
            Sprite sprite(&renderTexture.getTexture(), IntRect(region.xy.x, region.xy.y, region.size.x,
                    region.size.y));

            sprite.setPosition(pos + pen + region.bitmapTopLeft);

            pen.x += region.advanceX;

            renderWindow.draw(sprite);
        }
    }
}

Font::Font(Font&& other) : size(std::exchange(other.size, 0)), faceHeight(std::exchange(other.faceHeight, 0)),
                           renderTexture(std::exchange(other.renderTexture, RenderTexture())),
                           spriteShader(std::exchange(other.spriteShader, nullptr))
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
    spriteShader = std::exchange(rhs.spriteShader, nullptr);

    for(int i = 0; i < NUM_GLYPHS; ++i)
    {
        new (&regions[i]) GlyphRegion(std::exchange(rhs.regions[i], GlyphRegion()));
    }

    return *this;
}

bool Font::loadFaceFromLibrary(const void* fileBuffer, long long fileSize, FT_Face& face)
{
    int error = FT_New_Memory_Face(library, (const uchar*) fileBuffer, fileSize, 0, &face);
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
        utils::log("no charmap found");
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
