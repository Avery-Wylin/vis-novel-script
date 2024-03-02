#include <vector>
#include <stdexcept>
#include <glm/ext/matrix_transform.hpp>
#include "Text.h"
#include <iostream>
#include <cstring>
#include <algorithm>

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

void Text::generate(FontInfo &font){
    if(!needsGenerated)
        return;

    vector<float> pos;
    vector<float> uv;
    vector<uint32_t> id;
    
    width = 0;
    height = 0;
    
    int
    xoffset = 0,
    yoffset = 0;
    char c;
    bool isSelected;
    
    float x1,y1,x2,y2, u1,v1,u2,v2;
    
    GlyphInfo g;
    vertexCount = 0;
    
    for(int i = 0; i < text.length(); i++){
        c = text.at(i);
        if( c == '\n' ){
            xoffset = 0;
            yoffset += font.lineHeight;
            continue;
        }
        // else if(c == ' '){
        //      xoffset += spacing;
        //      continue;
        // }
        
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
        //bl
        pos.push_back( x1 );
        pos.push_back( y1 );
        uv.push_back( u1 );
        uv.push_back( v1 );
        uv.push_back( isSelected );
        //br
        pos.push_back( x2 );
        pos.push_back( y1 );
        uv.push_back( u2 );
        uv.push_back( v1 );
        uv.push_back( isSelected );
        //tr
        pos.push_back( x2 );
        pos.push_back( y2 );
        uv.push_back( u2 );
        uv.push_back( v2 );
        uv.push_back( 0 );

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
            //bl
            pos.push_back( x1 );
            pos.push_back( y1 );
            uv.push_back( u1 );
            uv.push_back( v1 );
            uv.push_back( 0 );
            //br
            pos.push_back( x2 );
            pos.push_back( y1 );
            uv.push_back( u2 );
            uv.push_back( v1 );
            uv.push_back( 0 );
            //tr
            pos.push_back( x2 );
            pos.push_back( y2 );
            uv.push_back( u2 );
            uv.push_back( v2 );
            uv.push_back( 0 );
            vertexCount += 4;
        }

        vertexCount += 4;

        xoffset += spacing;
        if(width < xoffset)
            width = xoffset;
    }

    height = ( yoffset + font.lineHeight ) / font.resolution;
    width /= font.resolution;
    
    vao->load_attrb_float(Attribute::ATTRB_POS, 0, 0, 2, pos.size(), pos.data());
    vao->load_attrb_float(Attribute::ATTRB_UV, 0, 0, 3, uv.size(), uv.data());
    needsGenerated = false;
}

bool Text::empty(){
    return vao->vaoid==0;
}

void Text::bind(FontInfo &font){
    generate(font);
    vao->bind();
}


