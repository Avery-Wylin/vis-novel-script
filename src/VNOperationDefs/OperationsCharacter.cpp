#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP{


    void load_ops_character(){
        operation_map["character create"] = character_create;
        format_map[character_create] = {"character create"}; // TODO

        operation_map["character name"] = character_name;
        format_map[character_name] = {"character name"}; // TODO

        operation_map["say element"] = say_element;
        format_map[say_element] = {"say element ( text name ) -name"};

        operation_map["say start"] = say_start;
        format_map[say_start] = {"say start -character :"};

        operation_map["say continue"] = say_continue;
        format_map[say_continue] = {"say continue :"};

        operation_map["say finish"] = say_finish;
        format_map[say_finish] = {"say finish :"};

    }
};


void VNOP::character_create( func_args ) {
}

void VNOP::character_name( func_args ) {

}

void VNOP::say_element(func_args){
    if(args[0].value_string() == "text"){
        VNI::elem_text = args[1].value_string();
    }
    else if(args[0].value_string() == "name"){
        VNI::elem_name = args[1].value_string();
    }
}

void VNOP::say_start( func_args ) {
    if(!Menu::active)
        return;
    Element *e_name = Menu::active->element_map[VNI::elem_name];
    Element *e_text = Menu::active->element_map[VNI::elem_text];
    if(e_name){
        e_name->flags &= ~(Element::REVEAL_TEXT | Element::LEFT_ALIGN);
        e_name->set_text(args[0].value_string());
    }
    if(e_text){
        e_text->flags |= Element::REVEAL_TEXT | Element::LEFT_ALIGN;
        e_text->set_text(args[1].value_string());
    }

    // Call wait regardless (crazy skipping could happen with poorly set-up menus)
    VNI::wait();
}

void VNOP::say_continue( func_args ) {
    if(!Menu::active)
        return;
    Element *e_text = Menu::active->element_map[VNI::elem_text];
    if(e_text){
        e_text->flags |= Element::REVEAL_TEXT | Element::LEFT_ALIGN;
        e_text->append_text(args[0].value_string());
    }

    // Call wait regardless (crazy skipping could happen with poorly set-up menus)
    VNI::wait();
}

void VNOP::say_finish( func_args ) {

}
