#include "Element.h"
#include "ElementSet.h"
#include "GUI.h"

Element::Element() {};
Element::~Element() {
    if( owner ) {
        // This should be avoided because there is no way of assuring the parent exists still
        // Generally, elements should be removed from the set before the destructor is called
        printf( "Warning: An Element was deleted before its containing ElementSet.\n" );
        owner->remove( this );
    }

};

void Element::set_owner( ElementSet *owner ) {
    if( this->owner != nullptr ) {
        printf( "Can not reasign owner of an Element.\nUnlink all elements before destructor using remove_all()\n" );
        return;
    }

    this->owner = owner;
};


// When the element is clicked after another/none is selected
void Element::event_select( float x, float y ) {
    if( !( flags & ACTIVE ) )
        GUI::play_sound_select();

    flags |= ACTIVE;
}

// When the element is selected and another/none is selected
void Element::event_deselect() {
    flags &= ~ACTIVE;
};

// When the element is selected and clicked again
void Element::event_reselect( float x, float y ) {};

// When the element is selected and a key combo is pressed
void Element::event_keypress( int32_t key_value, uint8_t modifiers_value ) {};

void Element::event_char( char c ) {};

bool Element::is_active() {
    return flags & ACTIVE;
}

void Element::set_hidden( bool b ) {
    if( b )
        flags |= HIDDEN;
    else
        flags &= ~HIDDEN;
}

void Element::localize_coordinates( float &sx, float &sy ) {
    sx = ( sx - x ) / ( w / GUI::ratio );
    sy = ( sy - y ) / h;
}

bool Element::contains_point( float sx, float sy, float &lx, float &ly ) {

    float b = fmax( GUI::bevel, 1.0f / fmin( w, h ) );

    // Convert the screen coordinates into the local space
    float ux, uy;
    lx = sx;
    ly = sy;
    localize_coordinates( lx, ly );

    // Shift the local coordinates so the origin is in the center
    // Absolute the coordinates and subtract a half, this inverts the values,
    // Multiply by the bevel and width, this will make a larger negative number closer to the edge
    // Add back half,
    ux = w * b * ( abs( lx - .5 ) -  0.5 ) + 0.5;
    uy = h * b * ( abs( ly - .5 ) -  0.5 ) + 0.5;

    // Clamp any negative numbers to 0
    ux = fmax( 0, ux );
    uy = fmax( 0, uy );

    return  ux * ux + uy * uy < .25;
}


void Element::set_position( float x, float y ) {
    this->x = x;
    this->y = y;
};

void Element::set_size( float w, float h ) {
    this->w = w;
    this->h = h;
}

void Element::set_text( const std::string &s ) {
    reveal_amount = 0;
    if( text )
        text->set_text( s );
}

void Element::set_text( const char *s, uint32_t count ) {
    reveal_amount = 0;
    if( text )
        text->set_text( s, count );
}

void Element::append_text(const std::string &s){
    if( text ){
        reveal_amount = text->length();
        text->append( s );
    }
};

const std::string& Element::get_text() {
    if( text )
        return text->get_text();
}

bool Element::changed() {
    if( flags & CHANGED ) {
        flags &= ~CHANGED;
        return true;
    }
    else {
        return false;
    }
}

void Element::hide() {
    flags |= HIDDEN;
}

void Element::show() {
    flags &= ~HIDDEN;
}

void Element::get_vn_var( VNVariable &dest){
    switch( type ) {
        case NONE:
            // Leave untouched if no type
            return;

        case TEXT:
            // Set to match the text
            dest.set(get_text());
            return;

        case BUTTON:
            // Set to whether the button was clicked, this consumes the change
            dest.set(changed());
            return;

        case OPTION:
            // Set to the option selected value
            dest.set((int)((ElementOption*)this)->selected());
            return;

        case TOGGLE:
            // Set to whether the toggle is activated
            dest.set(is_active());
            return;

        case TEXT_INPUT:
            // Set to the text input
            dest.set(get_text());
            return;

        case NUM_INPUT:
            // If int format, set the variable to int, otherwise float
            if(((ElementNumInput*)this)->is_int_format())
                dest.set( ((ElementNumInput*)this)->get_int() );
            else
                dest.set( ((ElementNumInput*)this)->get_float() );
            return;

        case BAR:
            // Set to the bar float value
            dest.set( ((ElementBar*)this)->get_value());
            return;

        case KEY_CAPTURE:
            // Use the key capture string
            dest.set(((ElementKeyCapture*)this)->get_key());
            return;
    }
}

 void Element::set_vn_var(VNVariable &src ){
     switch( type ) {
        case BUTTON:
        case NONE:
            // Leave unchanged
            return;

        case TEXT:
            // Set to match the text
            set_text( src.value_string());
            return;

        case OPTION:
            // Set to the option selected value
            ((ElementOption*)this)->select_option( src.value_int());
            return;

        case TOGGLE:
            // Set to whether the toggle is activated
            if( src.value_bool())
                flags |= ACTIVE;
            else
                flags &= ~ACTIVE;
            return;

        case TEXT_INPUT:
            // Set to the text input
            set_text( src.value_string());
            return;

        case NUM_INPUT:
            ((ElementNumInput*)this)->set_value( src.value_float());
            return;

        case BAR:
            ((ElementBar*)this)->set_value( src.value_float());
            return;

        case KEY_CAPTURE:
            ((ElementKeyCapture*)this)->set_key( src.value_int());
            return;
    }
}

// BUTTON ------------------------------------------------------------------------------------------------------

void ElementButton::event_select( float x, float y ) {
    if( !( flags & CHANGED ) ) {
        GUI::play_sound_select();
    }

    flags |= CHANGED;
}

// Also trigger active on reselect
void ElementButton::event_reselect( float x, float y ) {
    event_select( x, y );
}

// OPTION ------------------------------------------------------------------------------------------------------

void ElementOption::event_reselect( float x, float y ) {
    GUI::play_sound_select();
    flags |= CHANGED;
    selected_option = ( selected_option + 1 ) % options.size();
    text->set_text( this->options[selected_option] );
}


void ElementOption::event_keypress( int32_t key, uint8_t modifier ) {
    switch( key ) {
        case GLFW_KEY_DOWN:
        case GLFW_KEY_LEFT:
            selected_option = ( ( options.size() + selected_option ) - 1 ) % options.size();
            text->set_text( this->options[selected_option] );
            GUI::play_sound_deselect();
            flags |= CHANGED;
            break;

        case GLFW_KEY_UP:
        case GLFW_KEY_RIGHT:
            selected_option = ( selected_option + 1 ) % options.size();
            text->set_text( options[selected_option] );
            GUI::play_sound_select();
            flags |= CHANGED;
            break;

        default:
            break;
    }
}

void ElementOption::select_option( uint32_t index ) {
    selected_option = index % options.size();;
    flags |= CHANGED;
    text->set_text( options[selected_option] );
}

void ElementOption::set_option( uint32_t index, string option ) {
    if( index >= options.size() )
        return;

    options[index] = option;

    if( index == selected_option )
        text->set_text( options[selected_option] );
}

void ElementOption::set_options( std::initializer_list<string> list ) {
    options = list;
    selected_option %= list.size();
    text->set_text( options[selected_option] );
}

void ElementOption::resize_options( uint32_t count ) {
    options.resize( count );
    selected_option %= options.size();
}

void ElementOption::delete_option( uint32_t index ) {
    if( index >= options.size() || options.size() == 1 )
        return;

    if( selected_option >= index )
        selected_option--;

    options.erase( options.begin() + index );
}

// TOGGLE ------------------------------------------------------------------------------------------------------

// Toggle on select
void ElementToggle::event_select( float x, float y ) {
    if( !( flags & ACTIVE ) )
        GUI::play_sound_select();
    else
        GUI::play_sound_deselect();

    flags |= CHANGED;
    flags ^= ACTIVE;
}

// Toggle on reselect
void ElementToggle::event_reselect( float x, float y ) {
    event_select( x, y );
}

// TEXT INPUT -----------------------------------------------------------------------------------------------------

// Reset the cursor position on selection
void ElementTextInput::event_select( float x, float y ) {
    if( !( flags & ACTIVE ) )
        GUI::play_sound_select();

    text->set_cursor( text->get_text().size() );
    flags |= ACTIVE;
}

// Unselect text when deselected
void ElementTextInput::event_deselect() {
    text->deselect();
    flags &= ~ACTIVE;
}

// Select all text when reselected
void ElementTextInput::event_reselect( float x, float y ) {
    text->select_all();
}

// Additional text manipulation keys
void ElementTextInput::event_keypress( int32_t key, uint8_t modifier ) {
    switch( key ) {
        case GLFW_KEY_BACKSPACE:
            text->erase();
            flags |= CHANGED;
            break;

        case GLFW_KEY_LEFT:
            if( modifier & GLFW_MOD_SHIFT ) {
                text->select_more( -1 );
            }
            else {
                text->move_cursor( -1 );
            }

            break;

        case GLFW_KEY_RIGHT:
            if( modifier & GLFW_MOD_SHIFT ) {
                text->select_more( 1 );
            }
            else {
                text->move_cursor( 1 );
            }

            break;
    }
}

// Text insertion when typing
void ElementTextInput::event_char( char c ) {
    if( text->get_text().size() < max_length || text->get_selection_text() > 0 ) {
        text->overwrite( c );
        flags |= CHANGED;
    }
}

// NUM INPUT ------------------------------------------------------------------------------------------------------

// Reset the cursor position on selection
void ElementNumInput::event_select( float x, float y ) {
    if( !( flags & ACTIVE ) )
        GUI::play_sound_select();

    text->set_cursor( text->get_text().size() );
    flags |= ACTIVE;
}

// Unselect text when deselected
void ElementNumInput::event_deselect() {
    parse_value();
    text->deselect();
    flags &= ~ACTIVE;
}

// Select all text when reselected
void ElementNumInput::event_reselect( float x, float y ) {
    text->select_all();
}

// Additional text manipulation keys
void ElementNumInput::event_keypress( int32_t key, uint8_t modifier ) {
    switch( key ) {
        case GLFW_KEY_BACKSPACE:
            text->erase();
            break;

        case GLFW_KEY_LEFT:
            if( modifier & GLFW_MOD_SHIFT ) {
                text->select_more( -1 );
            }
            else {
                text->move_cursor( -1 );
            }

            break;

        case GLFW_KEY_RIGHT:
            if( modifier & GLFW_MOD_SHIFT ) {
                text->select_more( 1 );
            }
            else {
                text->move_cursor( 1 );
            }

            break;
    }
}

// Text insertion when typing
void ElementNumInput::event_char( char c ) {
    if( ( c < '0' || c > '9' ) && !( c == '.' || c == '-' ) )
        return;

    if( text->get_text().size() < max_chars || text->get_selection_text() > 0 ) {
        text->overwrite( c );
    }
}

// BAR ------------------------------------------------------------------------------------------------------

// Only change value on reselection
void ElementBar::event_reselect( float x, float y ) {
    if( t <= x )
        GUI::play_sound_select();
    else
        GUI::play_sound_deselect();

    t = x;
    update_value();
};

// Additional key manipulation
void ElementBar::event_keypress( int32_t key, uint8_t modifier ) {
    switch( key ) {
        case GLFW_KEY_LEFT:
            if( modifier & GLFW_MOD_SHIFT ) {
                t -= .01f;
            }
            else {
                t -= .1f;
            }

            t = fmin( fmax( t, 0 ), 1 );
            update_value();
            GUI::play_sound_deselect();
            break;

        case GLFW_KEY_RIGHT:
            if( modifier & GLFW_MOD_SHIFT ) {
                t += .01f;
            }
            else {
                t += .1f;
            }

            t = fmin( fmax( t, 0 ), 1 );
            update_value();
            GUI::play_sound_select();
            break;
    }
}

// KEY CAPTURE ------------------------------------------------------------------------------------------------------

void ElementKeyCapture::update_label() {
    std::string new_label = label + ": ";

    // Modifier Values
    if( modifiers & GLFW_MOD_CONTROL )
        new_label.append( "ctrl " );

    if( modifiers & GLFW_MOD_ALT )
        new_label.append( "alt " );

    if( modifiers & GLFW_MOD_SHIFT )
        new_label.append( "shift " );

    if( key > 96 )
        switch( key ) {
            case GLFW_KEY_ENTER:
                new_label.append( "enter " );
                break;

            case GLFW_KEY_TAB:
                new_label.append( "tab" );
                break;

            case GLFW_KEY_BACKSPACE:
                new_label.append( "backspace" );
                break;

            case GLFW_KEY_INSERT:
                new_label.append( "insert" );
                break;

            case GLFW_KEY_DELETE:
                new_label.append( "delete" );
                break;

            case GLFW_KEY_RIGHT:
                new_label.append( "right" );
                break;

            case GLFW_KEY_LEFT:
                new_label.append( "left" );
                break;

            case GLFW_KEY_DOWN:
                new_label.append( "down" );
                break;

            case GLFW_KEY_UP:
                new_label.append( "up" );
                break;

            case GLFW_KEY_PAGE_UP:
                new_label.append( "page up" );
                break;

            case GLFW_KEY_PAGE_DOWN:
                new_label.append( "page down" );
                break;

            case GLFW_KEY_HOME:
                new_label.append( "home" );
                break;

            case GLFW_KEY_END:
                new_label.append( "end" );
                break;

            case GLFW_KEY_CAPS_LOCK:
                new_label.append( "caps lock" );
                break;

            case GLFW_KEY_SCROLL_LOCK:
                new_label.append( "scroll lock" );
                break;

            case GLFW_KEY_NUM_LOCK:
                new_label.append( "num lock" );
                break;

            case GLFW_KEY_PRINT_SCREEN:
                new_label.append( "print screen" );
                break;

            case GLFW_KEY_PAUSE:
                new_label.append( "pause" );
                break;
        }

    else if( key == GLFW_KEY_UNKNOWN )
        new_label.append( "none" );

    // Print space character (it is not visible)
    else if( key == GLFW_KEY_SPACE )
        new_label.append( "space" );

    // Assume the rest of ASCII is printable
    else
        new_label.push_back( key );

    text -> set_text( new_label );
}

bool ElementKeyCapture::set_key( int32_t key_value, uint8_t modifiers_value ) {
    // If the key is escape, then capture it and set the keybind to none
    if( key_value == GLFW_KEY_ESCAPE )
        key_value = GLFW_KEY_UNKNOWN;

    // Exit if the input values are invalid (non-ASCII) or nav keys, exclude escape
    else if( key_value < 32 || ( key_value > 96 && key_value < 257 ) || ( key_value > 284 ) )
        return false;

    // Return if there is no change
    if( key == key_value && modifiers == modifiers_value )
        return false;

    // If there is a change mark it and update the text
    key = key_value;
    modifiers = modifiers_value;
    update_label();
    return true;
}

void ElementKeyCapture::event_keypress( int32_t key_value, uint8_t modifiers_value ) {
    if( set_key( key_value, modifiers_value ) ) {
        flags |= CHANGED;

        if( owner )
            owner->deselect();
    }
};
