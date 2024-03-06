#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP{


    void load_ops_character(){

        operation_map["say element"] = say_element;
        format_map[say_element] = {"say element ( text name ) -element"};

        operation_map["say start"] = say_start;
        format_map[say_start] = {"say start | -char_name :"};

        operation_map["say continue"] = say_continue;
        format_map[say_continue] = {"say continue :"};

        operation_map["say print"] = say_print;
        format_map[say_print] = {"say print -name | vars..."};
    }
};

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
    Element *e_text = Menu::active->element_map[VNI::elem_text];
    Element *e_name = Menu::active->element_map[VNI::elem_name];

    // If a name is given, show on the name element
    if(e_name){
        if(args.size() == 2 && !args[0].value_string().empty()){
            e_name->flags &= ~(Element::REVEAL_TEXT | Element::LEFT_ALIGN | Element::HIDDEN);
            e_name->set_text(args[0].value_string());
        }
        else{
            e_name->flags |= (Element::HIDDEN);
        }
    }

    // Show on the text element
    if(e_text){
        e_text->flags |= Element::REVEAL_TEXT | Element::LEFT_ALIGN;
        e_text->set_text(args.back().value_string());
    }

    // Call wait regardless ( otherwise crazy skipping could happen with poorly set-up menus)
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

    // Call wait regardless ( otherwise crazy skipping could happen with poorly set-up menus)
    VNI::wait();
}

void VNOP::say_print( func_args ) {
    if(!Menu::active)
        return;
    Element *e_text = Menu::active->element_map[VNI::elem_text];
    Element *e_name = Menu::active->element_map[VNI::elem_name];

    // If a name is given, show on the name element
    if(e_name){
        if(!args[0].value_string().empty()){
            e_name->flags &= ~(Element::REVEAL_TEXT | Element::LEFT_ALIGN | Element::HIDDEN);
            e_name->set_text(args[0].value_string());
        }
        else{
            e_name->flags |= (Element::HIDDEN);
        }
    }

    if(e_text){
        e_text->flags |= Element::REVEAL_TEXT | Element::LEFT_ALIGN;
        e_text->text->set_text("");
        for(uint8_t i = 1; i < args.size()-1; ++i){
            e_text->append_text(args[i].value_string()+" ");
        }
        if(args.size()>1)
            e_text->append_text(args.back().value_string());

        // Reset the reveal amount
        e_text->reveal_amount = 0;
    }

    // Call wait regardless ( otherwise crazy skipping could happen with poorly set-up menus)
    VNI::wait();
}
