#ifndef ELEMENT_H
#define ELEMENT_H

#include "definitions.h"
#include "Text.h"
#include <vector>
#include <memory>
#include "VNVariable.h"

// Forward Declare
class ElementSet;

enum ElementType : uint8_t {
    NONE,       // Undeclared type
    TEXT,       // Simple text container, no interaction
    BUTTON,     // Can be enabled but only disabled by code
    OPTION,     // Switches between a list of options
    TOGGLE,     // Can be enabled/diabled by clicking again
    TEXT_INPUT, // Text that can be selected and edited
    NUM_INPUT,  // Number that can be selected and edited
    BAR,        // Select a float value based on x position
    KEY_CAPTURE // Capture whichever key is pressed
};

struct Element {
        static const uint8_t
        ACTIVE = 1,         // Determines if the element is selected or enabled, renders in different colors
        CHANGED = 2,        // Whether the element has changed since it was last read
        HIDDEN = 4,         // Hides the element when rendering and selecting
        DISABLED = 8,       // Disables selection
        LEFT_ALIGN = 16,    // Aligns display text leftmost, otherwise centered
        HIGHLIGHT = 32,     // Changes the line around the element to emphasize it
        REVEAL_TEXT = 64;   // Gradually reveals text when updated

        // Text *text = nullptr;
        std::unique_ptr<Text> text = nullptr; // Text drawn inside of an element
        ElementSet *owner = nullptr;          // The owner of this element
        uint8_t flags = 0;                    // Flags shared by all element types
        ElementType type = NONE;              // The element type
        float x = 0, y = 0, w = .2, h = .2;   // Position and dimensions of element

        std::string routine_file, routine_label;
        bool has_routine = false;
        uint32_t reveal_amount = 0;


        Element();
        virtual ~Element();

        // When the element is clicked after another/none is selected
        virtual void event_select( float x, float y );

        // When the element is selected and another/none is selected
        virtual void event_deselect();

        // When the element is selected and clicked again
        virtual void event_reselect( float x, float y );

        // When the element is selected and a key combo is pressed
        virtual void event_keypress( int32_t key_value, uint8_t modifiers_value );

        virtual void event_char( char c );
        bool is_active();
        void set_hidden( bool b );
        void localize_coordinates( float &sx, float &sy );
        bool contains_point( float sx, float sy, float &lx, float &ly );
        void set_owner( ElementSet *owner );
        void set_position( float x, float y );
        void set_size( float w, float h );
        void set_text( const std::string &s );
        void append_text( const std::string &s);
        void set_text( const char *s, uint32_t count );
        const std::string& get_text();
        bool changed();
        void hide();
        void show();
        void get_vn_var(VNVariable &dest);
        void set_vn_var(VNVariable &dest);

        Element& operator=( Element &src ) {
            text = std::make_unique<Text>( Text() );
            set_text(src.get_text());
            owner = nullptr;
            flags = src.flags;
            type = src.type;
            x = src.x;
            y = src.y;
            w = src.w;
            h = src.h;
        }

        Element(Element &src){
            text = std::make_unique<Text>( Text() );
            set_text(src.get_text());
            owner = nullptr;
            flags = src.flags;
            type = src.type;
            x = src.x;
            y = src.y;
            w = src.w;
            h = src.h;
        }
};

// Simple non-selectable text
class ElementText : public Element {

    public :
        ElementText() {
            type = TEXT;
            text = std::make_unique<Text>(Text());
            text->set_text( "Text", 0 );
            flags |= DISABLED;
            flags |= LEFT_ALIGN;
            flags |= REVEAL_TEXT;
        }
};

// A Button that is enabled by a click and disabled when the if state is read
class ElementButton : public Element {

    public :

        ElementButton() {
            type = BUTTON;
            text = std::make_unique<Text>(Text());
            text->set_text( "Button", 0 );
        }

        virtual void event_select( float x, float y ) override;
        virtual void event_reselect( float x, float y ) override;
        virtual void event_deselect() override {};
};

// Cycles between options when clicked
class ElementOption : public Element {
        uint8_t selected_option;
        std::vector<std::string> options;
    public :

        ElementOption() {
            type = OPTION;
            text = std::make_unique<Text>(Text());
            options = {"Option 1", "Option 2", "Option 3"};
            selected_option = 0;
            text->set_text( this->options[selected_option] );
        }

        void event_reselect( float x, float y ) override;
        void event_keypress( int32_t key, uint8_t modifier ) override;
        void select_option( uint32_t index );
        void set_option( uint32_t index, string option );
        void set_options( std::initializer_list<string> list );
        void resize_options( uint32_t count );
        void delete_option( uint32_t index );

        uint32_t option_count() {return options.size();}
        uint32_t selected() {return selected_option;}
};

// Turns on when clicked, turns off when clicked again, retains active state
class ElementToggle : public Element {
    public:

        ElementToggle() {
            type = TOGGLE;
            text = std::make_unique<Text>(Text());
            text->set_text( "Toggle", 0 );
        }

        void event_select( float x, float y ) override;
        void event_reselect( float x, float y ) override;
        void event_deselect() override {};

};

// Allows for typing a single line of text
class ElementTextInput : public Element {
        uint8_t max_length;

    public:
        ElementTextInput() {
            type = TEXT_INPUT;
            text = std::make_unique<Text>(Text());
            text->set_text( "Text Input", 0 );
            flags |= LEFT_ALIGN;
            max_length = 255;
        }

        void event_select( float x, float y ) override;
        void event_deselect() override;
        void event_reselect( float x, float y ) override;
        void event_keypress( int32_t key, uint8_t modifier ) override;
        void event_char( char c ) override;
};

// Allows for typing a number
// NOTE This is not very precise, still has rounding errors
class ElementNumInput : public Element {
        static const uint8_t max_chars = 8;
        float value = 0;
        bool is_int =  false;

        void parse_value() {
            float new_val =  value;

            try {
                new_val = std::stof( text->get_text() );
            }
            catch( std::invalid_argument &ex ) {
            }

            value = new_val;
            flags |= CHANGED;
            update_text();
        }

        void update_text() {
            char s[max_chars + 1];

            if( is_int ) {
                snprintf( s, max_chars + 1, "%d", ( int )value );
            }
            else
                snprintf( s, max_chars + 1, "%.4f", value );

            value = std::stof( s );

            text->set_text( s, 0 );
        }

    public:

        ElementNumInput() {
            type = TEXT_INPUT;
            text = std::make_unique<Text>(Text());
            flags |= LEFT_ALIGN;
            parse_value();
        }

        // Reset the cursor position on selection
        void event_select( float x, float y ) override;

        // Unselect text when deselected
        void event_deselect() override;

        // Select all text when reselected
        void event_reselect( float x, float y ) override;

        // Additional text manipulation keys
        void event_keypress( int32_t key, uint8_t modifier ) override;

        // Text insertion when typing
        void event_char( char c ) override;

        void set_value( float v ) {
            value = v;
            update_text();
        }

        void set_integer( bool b ) {
            is_int = b;
            parse_value();
        }

        bool is_int_format() {
            return is_int;
        }

        float get_float() {
            return value;
        }

        int get_int() {
            return ( int )value;
        }
};

// A bar that stores a float value between min and max
class ElementBar : public Element {
        static const uint8_t max_characters = 8;
        float value, min, max, t;
        string label = "Value";

        void update_value() {
            value = ( 1 - t ) * min + t * max;
            char num_label[max_characters];
            snprintf( num_label, max_characters, "%.2f", value );
            string new_label = label + ": " + num_label;
            text->set_text( new_label );
            flags |= CHANGED;
        }

    public:
        ElementBar() {
            type = BAR;
            text = std::make_unique<Text>(Text());
            value = 0;
            t = 0;
            min = 0;
            max = 1;
            update_value();
        }

        // Only change value on reselection
        void event_reselect( float x, float y ) override;

        // Additional key manipulation
        void event_keypress( int32_t key, uint8_t modifier ) override;

        void set_label( string s ) {
            label = s;
            update_value();
        }

        void set_range( float min_val, float max_val ) {
            min = min_val;
            max = max_val;
            update_value();
        }

        void set_value( float v ) {
            t = fmax( fmin( ( v - min ) / ( max - min ), 1 ), 0 );
            update_value();
        }

        float get_normalized_value() {
            return t;
        }

        float get_value() {
            return value;
        }
};

// Displays a key combination typed while selected, then deselects itself
class ElementKeyCapture : public Element {
        int32_t key = GLFW_KEY_UNKNOWN;
        uint8_t modifiers = 0;
        string label = "Keybind";

        void update_label();

    public:

        ElementKeyCapture() {
            type = KEY_CAPTURE;
            text = std::make_unique<Text>(Text());
            update_label();
        };

        bool set_key( int32_t key_value, uint8_t modifiers_value );
        void event_keypress( int32_t key_value, uint8_t modifiers_value ) override;

        void set_label( string s ) {
            label = s;
            update_label();
        }

        int32_t get_key() {
            return key;
        }

        void set_key(int32_t c){
            key = c;
            update_label();
        }

        uint8_t get_mods() {
            return modifiers;
        }
};

#endif // ELEMENT_H
