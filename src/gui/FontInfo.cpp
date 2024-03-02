#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include "FontInfo.h"

using std::string;
using std::ifstream;
using std::cout;
using std::endl;

FontInfo::FontInfo() {
    resolution = 256;
    lineHeight = 0;
    glyphCount = 0;
    glyphs = std::map<char, GlyphInfo>();
}

FontInfo::FontInfo( FontInfo const &a ) {
    resolution = a.resolution;
    lineHeight = a.lineHeight;
    glyphCount = a.glyphCount;
    glyphs = a.glyphs;
}

FontInfo::~FontInfo() {
}

void FontInfo::free_texture(){
    fontTexture.free();
}

void FontInfo::load( string filename ) {
    ifstream input;
    input.open( (std::string)DIR_FONTS + filename + ".fnt" );
    fontTexture.load_png( filename, GL_LINEAR , GL_CLAMP, GL_RED);
    if( !input.is_open() ) {
        printf( "Failed to open font %s", filename.c_str() );
        return;
    }

    // Ignore info line
    input.ignore( 256, '\n' );

    // Read common data
    // ignore "common"
    input.ignore( 256, ' ' );

    // lineHeight
    input.ignore( 256, '=' );
    input >> lineHeight;

    // resolution
    input.ignore( 256, '=' );
    input >> resolution;

    // glyph count
    input.ignore( 256, '=' );
    input >> glyphCount;

    // start reading data
    int id;

    for( int i = 0; i < glyphCount; i++ ) {
        GlyphInfo glyph;

        // id
        input.ignore( 256, '=' );
        input >> id;

        // x
        input.ignore( 256, '=' );
        input >> glyph.x;

        // y
        input.ignore( 256, '=' );
        input >> glyph.y;

        // width
        input.ignore( 256, '=' );
        input >> glyph.width;

        // height
        input.ignore( 256, '=' );
        input >> glyph.height;

        // xoffset
        input.ignore( 256, '=' );
        input >> glyph.xoffset;

        // yoffset
        input.ignore( 256, '=' );
        input >> glyph.yoffset;

        // xadvance
        input.ignore( 256, '=' );
        input >> glyph.xadvance;

        glyphs.emplace( ( char )id, glyph );
    }

    input.close();
}
