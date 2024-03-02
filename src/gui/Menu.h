#ifndef MENU_H
#define MENU_H

#include <unordered_map>
#include <string>
#include "definitions.h"
#include "GUI.h"
#include "VNVariable.h"
#include <memory>

struct MenuBase{
    ElementSet element_set;
    std::unordered_map<std::string, Element*> element_map;

    MenuBase(){};
    virtual ~MenuBase(){};
    void open();
    void close();
    void update();

    template<typename T>
    void add_element(std::string name, T e){
        // Must be a type of Element
        if( !std::is_base_of<Element, T>::value )
            return;

        // Get the unique pointer reference
        Element *&elem = element_map[name];


        // If the unique ptr is not empty, delete it
        if( elem ) {
            // It is ok to call the destructor here since the element set is assured to exist
            delete elem;
        }

        elem = new T(e);

        // Set ownership
        element_set.add( elem );
    }

    void remove_element(std::string name);
    void clear();
    void get_value(std::string element_name, VNVariable& dest);
};

namespace Menu{
    extern MenuBase *active;
    extern std::unordered_map<std::string, MenuBase> menu_map;
    void activate( std::string menu_name );
    void create(std::string menu_name);
    void deactivate();
    void update();
};


#endif // MENU_H
