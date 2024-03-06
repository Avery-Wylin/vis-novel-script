#include "GUI.h"
#include "../graphics/Shader.h"

#include <iostream>

namespace GUI {
    FontInfo font;
    Shader text_shader, background_shader;
    VAO quad_vao;
    float ratio = 2.0f, bevel = 30.0f, volume = .8, max_text_size = .3;
    ElementSet *selection = nullptr;
    SoundBuffer sound_select, sound_hover;
    SoundSource sound_source;
    ElementColor colors;
}

void GUI::init_assets() {
    font.load( "liberation-mono" );
    text_shader.load( "text2D" );
    background_shader.load( "gui_simple" );
    float pos[8] = {0, 0, 1, 0, 1, 1, 0, 1};
    float uv[8] = {0, 0, 1, 0, 1, 1, 0, 1};
    quad_vao.load_attrb_float( Attribute::ATTRB_POS, 0, 0, 2, 8, pos );
    quad_vao.load_attrb_float( Attribute::ATTRB_UV, 0, 0, 2, 8, uv );
    sound_select.load( "element_click" );
    sound_hover.load( "element_hover" );
    sound_source.allocate();
}

void GUI::close_assets() {
    text_shader.free();
    background_shader.free();
    quad_vao.free();
    font.free_texture();
    sound_select.free();
    sound_hover.free();
    sound_source.free();
}

void GUI::select_set( ElementSet *es ) {
    selection = es;
}

void GUI::deselect_set() {
    selection = nullptr;
}

void GUI::draw() {
    if( selection ) {
        selection->draw();
    }
}

void GUI::char_input( char c ) {
    if( selection )
        selection->char_input( c );
}

void GUI::highlight(float x, float y){
    if(selection)
        selection->highlight(x,y);
}

void GUI::select( float x, float y ) {
    if( selection )
        selection->select( x, y );
}

void GUI::key_input( uint32_t key_value, uint8_t modifiers_value ) {
    if( selection )
        selection->key_input( key_value, modifiers_value );
}

void GUI::play_sound_select() {
    if( sound_source.is_playing() )
        return;

    sound_source.set_sound( sound_select );
    sound_source.set_volume( volume );
    sound_source.set_pitch( 1 );
    sound_source.play();
}

void GUI::play_sound_hover(){
    if( sound_source.is_playing() )
        return;

    sound_source.set_sound( sound_hover );
    sound_source.set_volume( volume );
    sound_source.set_pitch( 1 );
    sound_source.play();
}

void GUI::play_sound_deselect() {
    if( sound_source.is_playing() )
        return;

    sound_source.set_sound( sound_select );
    sound_source.set_volume( volume );
    sound_source.set_pitch( .8 );
    sound_source.play();
}


