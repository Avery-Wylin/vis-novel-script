#ifndef GUI_H
#define GUI_H

#include <vector>
#include <string>
#include <inttypes.h>
#include "Text.h"
#include "Audio.h"
#include <cglm/vec3.h>
#include "Element.h"
#include "ElementSet.h"


// Create a single namespace for the GUI interface
// All extern definitions are in the GUI.cpp file
namespace GUI {
    extern FontInfo font;
    extern Shader text_shader, background_shader;
    extern VAO quad_vao;
    extern float ratio, bevel, volume, max_text_size;
    extern ElementSet *selection;
    extern SoundSource sound_source;
    extern SoundBuffer sound_select;
    extern ElementColor colors;

    void init_assets();
    void close_assets();
    void select_set( ElementSet *es );
    void deselect_set();
    void draw();
    void char_input( char c );
    void highlight(float x, float y);
    void select( float x, float y );
    void key_input( uint32_t key_value, uint8_t modifiers_value );
    void play_sound_select();
    void play_sound_deselect();
    void play_sound_hover();
};


#endif // GUI_H
