#ifndef FONTINFO_H
#define FONTINFO_H

#include <string>
#include <map>
#include "../graphics/Texture.h"
#include "definitions.h"



struct GlyphInfo{
    int x;
    int y;
    int width;
    int height;
    int xoffset;
    int yoffset;
    int xadvance;
};

class FontInfo {
public:
    
    Texture fontTexture;
    float resolution = 256;
    int lineHeight = 0;
    int glyphCount;
    std::map<char,GlyphInfo> glyphs;
    
    FontInfo();
    FontInfo(FontInfo const &a);
    ~FontInfo();
    
    void load(std::string filename);
    void free_texture();

};

#endif /* FONTINFO_H */

