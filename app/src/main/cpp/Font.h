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
    int size = 0;
    int faceHeight = 0;
    RenderTexture renderTexture;
    //NOTE: Stack overflow danger?
    GlyphRegion regions[NUM_GLYPHS];
    FT_Library library = nullptr;
    Graphics* gfx = nullptr;
public:
    struct FontOptions
    {
        int size = 0;
    };
private:
    bool createGlyphRenderTextureAndMap(FT_Face& face);
    bool loadFaceFromLibrary(const void* fileBuffer, long long fileSize, FT_Face& face);
    void destructFace(FT_Face& face);
    bool loadGlyphIntoMap(char c, GlyphRegion& glyphRegion, FT_Face& face, Vector2i& xy);
public:
    bool loadFromFile(const String& filename, void* options);
    bool reloadFromFile(const String& filename);
public:
    Font() = default;
    Font(const Font& other) = delete;
    Font(Font&& other);
    Font& operator=(const Font& rhs) = delete;
    Font& operator=(Font&& rhs);
    long long getSize() const { return (sizeof(Font)); }
    void drawText(const String& text, const Vector2f& pos, Color color = Colors::White);
    void drawText(const String& text, const Vector2f& pos, int pixelSize, Color color = Colors::White);
    explicit operator bool() const;
};