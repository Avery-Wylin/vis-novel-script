#include "VNInterpreter.h"
#include "OperationDefs.h"
#include "GUI.h"

namespace VNOP{

    void load_ops_gui() {
        operation_map["menu create"] =  menu_create;
        format_map[menu_create]={"menu create -name"};

        operation_map["menu activate"] =  menu_activate;
        format_map[menu_activate]={"menu activate -name"};

        operation_map["menu compact"] =  menu_compact;
        format_map[menu_compact]={"menu compact ( left center ) -start -anchor -width -top -spacing_width -spacing_height"};

        operation_map["element create"] =  element_create;
        format_map[element_create]={"element create ( text button option toggle text-input num-input bar key-capture ) -name "};

        operation_map["element delete"] =  element_delete;
        format_map[element_delete]={"element delete -name"};

        operation_map["element enable"] =  element_enable;
        format_map[element_enable]={"element enable -name -bool"};

        operation_map["element value"] =  element_value;
        format_map[element_value]={"element value -name ( get set ) -value"};

        operation_map["element text"] =  element_text;
        format_map[element_text]={"element text -name ( get set ) -value"};

        operation_map["element size"] =  element_size;
        format_map[element_size]={"element size -name ( get set ) -width -height"};

        operation_map["element position"] =  element_position;
        format_map[element_position]={"element position -name ( get set ) -x -y"};

        operation_map["element routine"] =  element_routine;
        format_map[element_routine]={"element routine -name -label -file"};
    }

    /*
     * NOTE
     * Element operations should be separated per each element type except common operations
     * Get and Set should use the enum checker instead
     * Some options are only available for certain element types, so some special operations need made
     */
};

// gui menu create <name>
void VNOP::menu_create( func_args ) {
    ensure_args( 1 )
    Menu::create( args[0].value_string() );
}

// gui menu activate <name>
void VNOP::menu_activate( func_args ) {
    ensure_args( 1 )
    Menu::activate( args[0].value_string() );
}

// gui menu compact < center or left> <start-element> < left/center> <width/right> <top> <spacing> <line-space>
void VNOP::menu_compact( func_args ) {

    if( !Menu::active )
        return;

    if( args[0].value_string() == "center" ) {
        Menu::active->element_set.compact_center(
            std::max( args[1].value_int(), 0 ),  // start
            args[2].value_float(),               // center
            args[3].value_float(),               // width
            args[4].value_float(),               // top
            args[5].value_float(),               // spacing
            args[6].value_float()                // line spacing
        );
    }
    else if( args[0].value_string() == "left" ) {
        Menu::active->element_set.compact_center(
            std::max( args[1].value_int(), 0 ),  // start
            args[2].value_float(),               // left
            args[3].value_float(),               // right
            args[4].value_float(),               // top
            args[5].value_float(),               // spacing
            args[6].value_float()                // line spacing
        );
    }
}

// gui element create <type> <name>
void VNOP::element_create( func_args ) {

    if( !Menu::active )
        return;

    std::string type = args[0].value_string();

    if( type == "text" ) {
        Menu::active->add_element( args[1].value_string(), ElementText() );
    }
    else if( type == "button" ) {
        Menu::active->add_element( args[1].value_string(), ElementButton() );
    }
    else if( type == "option" ) {
        Menu::active->add_element( args[1].value_string(), ElementOption() );
    }
    else if( type == "toggle" ) {
        Menu::active->add_element( args[1].value_string(), ElementToggle() );
    }
    else if( type == "text-input" ) {
        Menu::active->add_element( args[1].value_string(), ElementTextInput() );
    }
    else if( type == "num-input" ) {
        Menu::active->add_element( args[1].value_string(), ElementNumInput() );
    }
    else if( type == "bar" ) {
        Menu::active->add_element( args[1].value_string(), ElementBar() );
    }
    else if( type == "key-capture" ) {
        Menu::active->add_element( args[1].value_string(), ElementKeyCapture() );
    }
}

// element delete <name>
void VNOP::element_delete( func_args ) {

    if( !Menu::active )
        return;

    Menu::active->remove_element( args[0].value_string() );
}

// element enable <name> <bool>
void VNOP::element_enable( func_args ) {

    if( !Menu::active )
        return;

    Element *e = Menu::active->element_map[args[0].value_string()];

    if( !e )
        return;

    e->set_hidden( !args[1].value_bool() );
}

// element value <name> < get or set > <var>
void VNOP::element_value( func_args ) {
    if( !Menu::active )
        return;

    Element *e = Menu::active->element_map[args[0].value_string()];

    if( !e )
        return;

    if( args[1].value_string() == "get" ) {
        if( args[2].var_id == 0 )
            return;

        e->get_vn_var( VNI::variables.at( args[2].var_id ) );
    }
    else if( args[1].value_string() == "set" ) {
        e->set_vn_var( args[2] );
    }
}

// element text <name> < get or set > <text>
void VNOP::element_text( func_args ) {
    if( !Menu::active )
        return;

    Element *e = Menu::active->element_map[args[0].value_string()];

    if( !e )
        return;

    if( args[1].value_string() == "get" ) {
        if( args[2].var_id == 0 )
            return;

        VNI::variables.at( args[2].var_id ).set( e->get_text() );
    }
    else if( args[1].value_string() == "set" ) {
        e->set_text( args[2].value_string() );
    }
}

// element size <name> < get or set > <width> <height>
void VNOP::element_size( func_args ) {
    if( !Menu::active )
        return;

    Element *e = Menu::active->element_map[args[0].value_string()];

    if( !e )
        return;

    if( args[1].value_string() == "get" ) {
        if( args[1].var_id )
            VNI::variables.at( args[1].var_id ).set( e->w );

        if( args[2].var_id )
            VNI::variables.at( args[2].var_id ).set( e->w );
    }
    else if( args[1].value_string() == "set" ) {
        e->set_size( args[2].value_float(), args[3].value_float() );
    }

}

void VNOP::element_position( func_args ) {
    if( !Menu::active )
        return;

    Element *e = Menu::active->element_map[args[0].value_string()];

    if( !e )
        return;

    if( args[1].value_string() == "get" ) {
        if( args[2].var_id )
            VNI::variables.at( args[2].var_id ).set( e->x );

        if( args[3].var_id )
            VNI::variables.at( args[3].var_id ).set( e->y );
    }
    else if( args[1].value_string() == "set" ) {
        e->set_position( args[2].value_float(), args[3].value_float() );
    }
}

// Set the operation of the element to the next line
// element routine <name> <file> <label>
void VNOP::element_routine( func_args ) {
    if( !Menu::active )
        return;

    Element *e = Menu::active->element_map[args[0].value_string()];

    if( !e )
        return;

    e->has_routine = true;
    e->routine_label = args[1].value_string();
    e->routine_file = args[2].value_string();
}
