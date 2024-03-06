#include <vector>
#include <stdexcept>
#include <glm/ext/matrix_transform.hpp>
#include "Text.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <regex>

using std::vector;

Text::Text(){
    vao = std::shared_ptr<VAO>(new VAO());
    needsGenerated = true;
}

Text::~Text() {
}

void Text::set_text(const std::string &text){
    this->text = text;
    needsGenerated = true;
    deselect();
}

void Text::set_text(const char *text, uint32_t length){
    length = length == 0 ? strlen(text) : length;
    this->text.assign(text,length<strlen(text)?length:strlen(text));
    needsGenerated = true;
    deselect();
}

void Text::append(char c){
    text.push_back(c);
    deselect();
    needsGenerated = true;
}

void Text::append(const std::string &s){
    text.append(s);
    deselect();
    needsGenerated = true;
}

void Text::overwrite(char c){
    if(selStop - selStart >= 1)
        erase();
    insert(c);
}

void Text::insert(char c){
    text.insert(text.begin()+selStart,c);
    selStart++;
    selStop=selStart;
    needsGenerated = true;
}

void Text::pop(){
    if(!text.empty())
    text.pop_back();

    deselect();
    needsGenerated = true;
}

void Text::erase(){
    if(selStart == selStop && selStart > 0){
        text.erase(selStart-1,1);
        selStart--;
        selStop = selStart;
    }
    else
        text.erase(selStart,selStop - selStart);
    selStop = selStart;
    needsGenerated = true;
}

void Text::set_cursor(int32_t position){
    if(position >= text.size() || position < 0)
        return;
    selStart = position;
    selStop = position;
    needsGenerated = true;
}

void Text::move_cursor(int32_t amount){
    if(selStart + amount < 0)
        selStart = 0;
    else if( selStart + amount > text.size())
        selStart = text.size();
    else
        selStart += amount;
    selStop = selStart;
    needsGenerated = true;
}

void Text::select_more(int32_t amount){
    if(amount < 0){
        if(selStart + amount > 0)
            selStart +=amount;
        else
            selStart = 0;
    }
    else{
        if(selStop + amount < text.size())
            selStop += amount;
        else
            selStop = text.size();
    }
    needsGenerated = true;
}

void Text::select_all(){
    selStart = 0;
    selStop = text.size();
    needsGenerated = true;
}

void Text::deselect(){
    selStart = text.size();
    selStop = text.size();
    needsGenerated = true;
}


std::string const& Text::get_text(){
    return text;
}

std::regex color_regex = std::regex(R"((#[0-9a-fA-F]{8})(.*$))");

void Text::generate(FontInfo &font){
    if(!needsGenerated)
        return;

    vector<float> pos;
    vector<float> uv;
    vector<uint32_t> color;
    vector<uint32_t> id;
    
    width = 0;
    height = 0;
    
    int
    xoffset = 0,
    yoffset = 0;
    char c;
    bool isSelected;
    
    float x1,y1,x2,y2, u1,v1,u2,v2;
    uint32_t text_color = 0x00000000;
    
    GlyphInfo g;
    vertexCount = 0;
    height = ( font.lineHeight ) / font.resolution;

    // x = approx. number of splits
    // (w/x)/(h*x) = ratio
    // w/(x*x*h) = ratio
    // w/ratio = x^2 * h
    // x = +sqrt(w/ratio/h)
    {
        float w = text.size()*spacing;
        float h = font.lineHeight;
        line_wrap = w/floor(sqrt(w/ideal_ratio/h));
    }
    
    for(uint32_t i = 0; i < text.length(); i++){
        c = text.at(i);

        // Color code
        if( c == '#' ) {
            if( std::regex_match( text.substr( i, text.size()-i ), color_regex ) ) {
                // Colors are stored 4 8-bit channels RGBA

                // Little Endian stuff ???
                uint8_t buffer[4];
                buffer[0] = ( uint8_t )std::strtoul( text.substr( i + 1, 2 ).c_str(), NULL, 16 );
                buffer[1] = ( uint8_t )std::strtoul( text.substr( i + 3, 2 ).c_str(), NULL, 16 );
                buffer[2] = ( uint8_t )std::strtoul( text.substr( i + 5, 2 ).c_str(), NULL, 16 );
                buffer[3] = ( uint8_t )std::strtoul( text.substr( i + 7, 2 ).c_str(), NULL, 16 );
                text_color = *( ( uint32_t * )buffer );
                i += 8;
                continue;
            }
        }


        // Escape Function symbols using '\'
        // This will skip to the next symbol without parsing it
        if( c == '\\' ){
            if(i<text.size()-1){
                switch(text[i+1]){
                    case '#': ++i; break;
                    case 't': c = '\t'; ++i; break;
                    case 'n': c = '\n'; ++i; break;
                }
            }
            else{
                continue;
            }
        }

        if( c == '\n' ){
            xoffset = 0;
            yoffset += font.lineHeight;
            continue;
        }
        if( line_wrap > 0 && (c == ' ' || c == '\t')){
            if(xoffset > line_wrap){
                xoffset = 0;
                yoffset += font.lineHeight;
                continue;
            }
        }
        try{
            g = font.glyphs.at(c);
        }
        catch(std::out_of_range &error){
            g = font.glyphs.at('?');
        }
        
        isSelected = i >= selStart && i < selStop;

        x1 = ( g.xoffset + xoffset ) / font.resolution;
        y1 = (-g.yoffset - g.height -yoffset ) / font.resolution;
        x2 = ( g.xoffset + xoffset + g.width ) / font.resolution;
        y2 = (-g.yoffset - yoffset ) / font.resolution;
        
        u1 = g.x /  font.resolution;
        v1 = ( g.y + g.height ) /  font.resolution;
        u2 = ( g.x + g.width ) /  font.resolution;
        v2 = ( g.y ) /  font.resolution;
        
        //tl
        pos.push_back( x1 );
        pos.push_back( y2 );
        uv.push_back( u1 );
        uv.push_back( v2 );
        uv.push_back( 0 );
        color.push_back(text_color);

        //bl
        pos.push_back( x1 );
        pos.push_back( y1 );
        uv.push_back( u1 );
        uv.push_back( v1 );
        uv.push_back( isSelected );
        color.push_back(text_color);

        //br
        pos.push_back( x2 );
        pos.push_back( y1 );
        uv.push_back( u2 );
        uv.push_back( v1 );
        uv.push_back( isSelected );
        color.push_back(text_color);

        //tr
        pos.push_back( x2 );
        pos.push_back( y2 );
        uv.push_back( u2 );
        uv.push_back( v2 );
        uv.push_back( 0 );
        color.push_back(text_color);


        // Draws a cursor (sorta excessive?)
        if( i == selStart) {
            g = font.glyphs.at('|');
            x1 = ( xoffset ) / font.resolution;
            y1 = ( -g.yoffset - g.height - yoffset ) / font.resolution;
            x2 = ( xoffset + g.width ) / font.resolution;
            y2 = ( -g.yoffset - yoffset ) / font.resolution;

            u1 = g.x /  font.resolution;
            v1 = ( g.y + g.height ) /  font.resolution;
            u2 = ( g.x + g.width ) /  font.resolution;
            v2 = ( g.y ) /  font.resolution;

            //tl
            pos.push_back( x1 );
            pos.push_back( y2 );
            uv.push_back( u1 );
            uv.push_back( v2 );
            uv.push_back( 0 );
            color.push_back(0x00000000);

            //bl
            pos.push_back( x1 );
            pos.push_back( y1 );
            uv.push_back( u1 );
            uv.push_back( v1 );
            uv.push_back( 0 );
            color.push_back(0x00000000);

            //br
            pos.push_back( x2 );
            pos.push_back( y1 );
            uv.push_back( u2 );
            uv.push_back( v1 );
            uv.push_back( 0 );
            color.push_back(0x00000000);

            //tr
            pos.push_back( x2 );
            pos.push_back( y2 );
            uv.push_back( u2 );
            uv.push_back( v2 );
            uv.push_back( 0 );
            color.push_back(0x00000000);
            vertexCount += 4;
        }

        vertexCount += 4;

        xoffset += spacing;

        // Update the maximum width
        if(width < xoffset/font.resolution)
            width = xoffset/font.resolution;
        height = ( yoffset + font.lineHeight ) / font.resolution;
    }

    
    vao->load_attrb_float(Attribute::ATTRB_POS, 0, 0, 2, pos.size(), pos.data());
    vao->load_attrb_float(Attribute::ATTRB_UV, 0, 0, 3, uv.size(), uv.data());
    vao->load_attrb_byte(Attribute::ATTRB_COL, 0, 0, 4, color.size()*4, true, color.data());
    needsGenerated = false;
}

bool Text::empty(){
    return vao->vaoid==0;
}

void Text::bind(FontInfo &font){
    generate(font);
    vao->bind();
}


