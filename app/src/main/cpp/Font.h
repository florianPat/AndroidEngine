#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "String.h"
#include "RenderTexture.h"
#include "Vector2.h"
#include "Window.h"

class Font
{
    struct GlyphRegion
    {
        Vector2f bitmapTopLeft;
        Vector2i xy;
        Vector2i size;
        uint advanceX;
    };
private:
    static constexpr int NUM_GLYPHS = '~' - ' ' + 1;
    uint32_t size = 0;
    uint32_t faceHeight = 0;
    RenderTexture renderTexture;
    //NOTE: Stack overflow danger?
    GlyphRegion regions[NUM_GLYPHS];
    FT_Library library = nullptr;
    Graphics* gfx = nullptr;
private:
    bool createGlyphRenderTextureAndMap(FT_Face& face);
    bool loadFaceFromLibrary(const void* fileBuffer, uint64_t fileSize, FT_Face& face);
    void destructFace(FT_Face& face);
    bool loadGlyphIntoMap(char c, GlyphRegion& glyphRegion, FT_Face& face, Vector2i& xy);
public:
    bool reloadFromFile(const String& filename);
public:
    Font(const String& filename, int size = 0);
    Font(const Font& other) = delete;
    Font(Font&& other);
    Font& operator=(const Font& rhs) = delete;
    Font& operator=(Font&& rhs);
    uint64_t getSize() const { return (sizeof(Font)); }
    void drawText(const String& text, const Vector2f& pos, Color color = Colors::White);
    void drawText(const String& text, const Vector2f& pos, int pixelSize, Color color = Colors::White);
    explicit operator bool() const;
};