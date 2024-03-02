#include "ElementSet.h"
#include "GUI.h"

void ElementSet::deselect() {
    if( selection )
        selection->event_deselect();

    selection = nullptr;
}

void ElementSet::highlight(float x, float y){
    for( Element *element : elements ) {
        if( element->flags & Element::DISABLED || element->flags & Element::HIDDEN )
            continue;

        float lx, ly;

        if( element->contains_point( x, y, lx, ly ) ) {
            if(!(element->flags & Element::HIGHLIGHT))
                GUI::play_sound_hover();
            element->flags |= Element::HIGHLIGHT;
        }
        else{
            element->flags &= ~Element::HIGHLIGHT;
        }
    }
}

void ElementSet::select( float x, float y ) {
    for( Element *element : elements ) {
        if( element->flags & Element::DISABLED || element->flags & Element::HIDDEN )
            continue;

        float lx, ly;

        if( element->contains_point( x, y, lx, ly ) ) {
            if( selection == element ) {
                bool_had_input = true;
                element->event_reselect( lx, ly );
            }
            else {
                if( selection )
                    selection->event_deselect();

                bool_had_input = true;
                element->event_select( lx, ly );
                selection = element;
            }

            return;
        }
    }

    deselect();
}

void ElementSet::add( Element *element ) {
    if( !element )
        return;

    if( element->owner ) {
        printf( "Element already has an owner.\n" );
        return;
    }

    element->set_owner( this );
    elements.push_back( element );
}

void ElementSet::remove( Element *element ) {
    for( unsigned int i = 0; i < elements.size(); ++i ) {
        if( elements[i] == element ) {
            element->owner = nullptr;
            elements.erase( elements.begin() + i );
            return;
        }
    }
}

void ElementSet::remove_all() {
    for( unsigned int i = 0; i < elements.size(); ++i ) {
        elements[i]->owner = nullptr;
    }

    elements.clear();
}

// NOTE Shader and VAO should be pre-bound before this call
void ElementSet::draw() {
    mat4 ortho;
    glm_ortho( 0, GUI::ratio, 0, 1, 0, 1, ortho );

    // Draw Backgrounds
    GUI::quad_vao.bind();
    Shader::bind( GUI::background_shader );
    Shader::uniformMat4f( UNIFORM_CAMERA, ortho );

    for( Element *e : elements ) {
        if( e->flags & Element::HIDDEN )
            continue;

        // Background
        Shader::uniformVec4f( UNIFORM_TRANSFORM, vec4{e->x * GUI::ratio, e->y, e->w, e->h} );
        Shader::uniformFloat( UNIFORM_FACTOR, fmax( GUI::bevel, 1.0f / fmin( e->w, e->h ) ) );
        if(e->is_active()){
            Shader::uniformVec3f( UNIFORM_COLOR,GUI::colors.background_active );
            Shader::uniformVec3f( UNIFORM_COLOR2,GUI::colors.outline_active );
        }
        else{
            Shader::uniformVec3f( UNIFORM_COLOR,GUI::colors.background_inactive );
            Shader::uniformVec3f( UNIFORM_COLOR2,GUI::colors.outline_inactive );
        }
        if(e->flags & Element::HIGHLIGHT)
            Shader::uniformVec3f( UNIFORM_COLOR2,GUI::colors.decorator_active );

        glDrawArrays( GL_QUADS, 0, 4 );

        // Addition decorator for bars
        if( e->type == BAR ) {
            float v = static_cast<ElementBar *>( e )->get_normalized_value();
            Shader::uniformVec4f( UNIFORM_TRANSFORM,
                vec4{e->x * GUI::ratio, e->y, e->w * v, e->h}
            );
            Shader::uniformVec3f( UNIFORM_COLOR, e->is_active() ? GUI::colors.decorator_active : GUI::colors.decorator_inactive );
            glDrawArrays( GL_QUADS, 0, 4 );
        }
    }

    // Draw Texts
    Shader::bind( GUI::text_shader );
    Shader::uniformMat4f( UNIFORM_CAMERA, ortho );
    GUI::font.fontTexture.bind( 0 );
    float scale;

    Shader::uniformVec3f( UNIFORM_COLOR2, GUI::colors.decorator_active );

    for( Element *e : elements ) {
        if( e->flags & Element::HIDDEN || !e->text )
            continue;

        e->text->bind( GUI::font );
        //only fill to 9 height
        scale = fmin( e->w / e->text->get_width(), e->h / e->text->get_height() * .9 );
        // Adjust scale into uniform sizes, this keeps the UI consistent
        scale = scale > .2 ? floor( scale * 10 ) / 10 : scale;
        // Text scale is pulled away from the edges by setting it to .9
        scale *= .9f;
        // Limit to max text size
        scale = fmin(scale, GUI::max_text_size);

        // Text transform is 2d screen position and scale
        Shader::uniformVec3f( UNIFORM_TRANSFORM, vec3{
            // If left align, use half the fraction of the box not filled (1-.9)/2, if center align, use half the difference
            e->x * GUI::ratio + ( e->flags & Element::LEFT_ALIGN ? .05f * e->w : 0.5f * ( e->w - e->text->get_width()*scale ) ),
            e->y + 0.5f * ( e->h + scale * e->text->get_height() ),
            scale}
        );
        if(e->is_active()){
            Shader::uniformVec3f( UNIFORM_COLOR, GUI::colors.text_active );
        }
        else{
            Shader::uniformVec3f( UNIFORM_COLOR, GUI::colors.text_inactive );
        }
        if(e->flags & Element::REVEAL_TEXT && e->reveal_amount < e->text->get_text().size()){
            e->reveal_amount++;
            Shader::uniformUint( UNIFORM_FACTOR, e->reveal_amount);
        }
        else{
            Shader::uniformUint( UNIFORM_FACTOR, UINT32_MAX-1);
        }

        glDrawArrays( GL_QUADS, 0, e->text->get_vertex_count() );
    }

    Shader::unbind();
}

void ElementSet::char_input( char c ) {
    if( !selection )
        return;

    bool_had_input = true;
    selection->event_char( c );
}

void ElementSet::key_input( uint32_t key_value, uint8_t modifiers_value ) {
    if( !selection )
        return;

    bool_had_input = true;
    selection->event_keypress( key_value, modifiers_value );
}

// Spaces elements apart horizontally
void ElementSet::compact( unsigned int start, float left, float right, float top, float spacing, float line_spacing ) {
    float w = left, h = top, max_height = 0;
    unsigned int line_start = start, line_end = start;

    for( unsigned int i = start; i < elements.size(); ++i ) {
        if( w + elements[i]->w / GUI::ratio > right ) {
            w = left;

            for( unsigned int i = line_start; i < line_end; ++i ) {
                elements[i]->y -= max_height;
            }

            h -= max_height + line_spacing;
            line_start = line_end;
            max_height = 0;
        }

        elements[i]->set_position( w, h );
        w += ( elements[i]->w + spacing ) / GUI::ratio;
        max_height = max_height < elements[i]->h ? elements[i]->h : max_height;
        ++line_end;
    }

    for( unsigned int i = line_start; i < line_end; ++i ) {
        elements[i]->y -= max_height;
    }
}

void ElementSet::compact_center( unsigned int start, float center, float max_width, float top, float spacing, float line_spacing ) {
    float w = center, h = top, line_width = 0, line_height = 0;
    unsigned int line_start = start, line_end = start;

    for( unsigned int i = start; i < elements.size(); ++i ) {
        // The line exceeds the maximum width, align all to center
        if( line_width > max_width ) {
            w = center;
            h -= line_height + line_spacing;

            // Push the line down by its height and shift over to center
            for( unsigned int j = line_start; j < line_end; ++j ) {
                elements[j]->x -= line_width / 2.0f;
                elements[j]->y -= line_height;
            }

            line_width = 0;
            line_height = 0;
            line_start = line_end;
        }

        elements[i]->set_position( w, h );
        line_width += ( elements[i]->w + spacing ) / GUI::ratio;
        w += ( elements[i]->w + spacing ) / GUI::ratio;
        line_height = line_height < elements[i]->h ? elements[i]->h : line_height;


        line_end++;
    }

    for( unsigned int i = line_start; i < line_end; ++i ) {
        elements[i]->x -= line_width / 2.0f;
        elements[i]->y -= line_height;
    }
}
