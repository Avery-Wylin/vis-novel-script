#ifndef ELEMENTSET_H
#define ELEMENTSET_H

#include "Element.h"

/*
 * Default colors used for an element set.
 * This is currently a soft biege and blue theme.
 */
struct ElementColor {
    vec3
    // Dark Default
    background_active = {.05,.05,.05},
    background_inactive = {.1,.1,.1},
    outline_active = {1,1,1},
    outline_inactive = {.2,.2,.2},
    text_active = {1,1,1},
    text_inactive = {.75,.75,.75},
    decorator_active = {.54,.54,.54},
    decorator_inactive = {.21, .21, .21};

    ElementColor(){
        theme_dark();
    }

    #define vec3_compose(dest, x, y, z); dest[0] = x; dest[1] = y; dest[2] = z;

    void theme_light(){
        vec3_compose( background_active, .8,.8,.8 );
        vec3_compose( background_inactive, 1, 1, 1 );
        vec3_compose( outline_active, .1, .12, .15  );
        vec3_compose( outline_inactive, 0, 0 ,0  );
        vec3_compose( text_active, 0, 0, .1 );
        vec3_compose( text_inactive, 0, 0, 0 );
        vec3_compose( decorator_active, .5, .65, .8 );
        vec3_compose( decorator_inactive, .6, .75, .9 );
    }

    void theme_dark(){
        vec3_compose( background_active, .05,.05,.05 );
        vec3_compose( background_inactive, .1,.1,.1 );
        vec3_compose( outline_active, 0, .6, .5 );
        vec3_compose( outline_inactive, 0, 0, 0 );
        vec3_compose( text_active, .75, .84, 1 );
        vec3_compose( text_inactive, 1,1,1 );
        vec3_compose( decorator_active, 0, .3, .7  );
        vec3_compose( decorator_inactive, 0, .2, .6 );
    }

};

/*
 * Keeps track of elements as well as providing a theme and operations.
 * Element sets that are active are drawn and input is passed from the global GUI functions.
 */
class ElementSet {
        // NOTE elements are allocated elsewhere and are not cleaned up by the element set
        std::vector<Element *> elements;
        Element *selection = nullptr;
        bool bool_had_input = false;

    public:

        void deselect();
        void select( float x, float y );
        void highlight( float x, float y );
        void add( Element *element );
        void remove( Element *element );
        void remove_all();
        void draw();
        void char_input( char c );
        void key_input( uint32_t key_value, uint8_t modifiers_value );
        void compact( uint32_t start, float left, float right, float top, float spacing, float line_spacing );
        void compact_center( uint32_t start, float center, float max_width, float top, float spacing, float line_spacing );
        inline const std::vector<Element*> &get_elements(){return elements;}
        inline uint32_t size() {return elements.size();}
        inline bool had_input() {
            if( bool_had_input ) {
                bool_had_input = false;
                return true;
            }
            else {
                return false;
            }
        }
        inline Element* get_selection(){return selection;}
};

#endif // ELEMENTSET_H
