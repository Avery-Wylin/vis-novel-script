#include "Menu.h"
#include "VNInterpreter.h"

namespace Menu {

    MenuBase *active = nullptr;
    std::unordered_map<std::string, MenuBase> menu_map;

    void create( std::string menu_name){
        menu_map[menu_name] = MenuBase();
    }

    void activate( std::string menu_name ){
        try{
            MenuBase *m = &menu_map.at(menu_name);
            if( m != active ) {
                active->close();
                active = m;
                GUI::select_set( &( m->element_set ) );
                m->open();
            }
        }
        catch(std::out_of_range &oor){
            printf("Menu %s does not exit.", menu_name.c_str());
        }
    };

    void deactivate() {
        if( !active )
            return;

        active->close();
        GUI::deselect_set();
        active = nullptr;
    };

    void update() {
        if( active ) {
            active->update();
        }
    }
};

// Called when a menu is activated
void  MenuBase::open(){

}

// Called when a menu is deactivated, or before another is activated
void  MenuBase::close(){

}

// Called every frame, but may ignore unless input is detected
void  MenuBase::update(){
    static VNInterpreter element_interpreter;

    if(!element_set.had_input())
        return;

    // Routines may add/remove elements, only one routine can be called per-update
    // This will remove the need to iterate over a changing list
    for(Element *e : element_set.get_elements()){
        if(e->changed()){
            if(e->has_routine)
                element_interpreter.start_routine(e->routine_file,e->routine_label);
            return;
        }
    }
}

// Extract the value of an element
void MenuBase::get_value(std::string element_name, VNVariable& dest){
    Element *&elem = element_map[element_name];

    // Leave the variable untouched if not found
    if(!elem)
        return;

    elem->get_vn_var(dest);
}



void MenuBase::remove_element(std::string name){
    // The destructor should handle ownership
    if(element_map["name"])
        delete element_map["name"];
    element_map["name"] = nullptr;
    element_map.erase(name);
}

void MenuBase::clear(){
    // Remove ownership before deletion
    element_set.remove_all();

    // Remove pointers left in the map
    for(auto &keypair : element_map){
        if(keypair.second){
            delete keypair.second;
        }
    }
    element_map.clear();
}
