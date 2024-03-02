#include "VNOperation.h"
#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP {

    void load_ops_model() {
        operation_map["model create"] = model_create;
        operation_map["model clear"] = model_clear;
        operation_map["model load"] = model_load;
        operation_map["model append"] = model_append;
        operation_map["model mix"] = model_mix;
        operation_map["model copy"] = model_copy;
        operation_map["model shader"] = model_shader;
    }
}



// model create <name>
void VNOP::model_create( func_args ) {
    exact_args( 1 )
    VNAssets::create_model( args[0].value_string() );
}

// model clear <name>
void VNOP::model_clear( func_args ) {
    exact_args( 1 )
    ModelContainer *m = VNAssets::get_model( args[0].value_string());

    if( !m )
        return;

    m->mesh.clear();
}

// model load <name> <filename>
void VNOP::model_load( func_args ) {
    exact_args( 2 )
    ModelContainer *m = VNAssets::get_model( args[0].value_string() );
    if( m ){
        m->mesh.append_PLY( args[1].value_string() );
    }
}

// model append <src> <dest>
void VNOP::model_append( func_args ) {
    exact_args( 2 )
    ModelContainer *src = nullptr, *dest = nullptr;
    src = VNAssets::get_model( args[0].value_string() );
    src = VNAssets::get_model( args[1].value_string() );
    if(!src){
        VNDebug::runtime_error("Source model not defined",args[0].value_string(),vni);
        return;
    }

    if(!dest){
        VNDebug::runtime_error("Destination model not defined",args[1].value_string(),vni);
        return;
    }

    dest->mesh.append_mesh( src->mesh );
}

// model mix <dest> <src> <other> <pos norm color uv> <factor>
void VNOP::model_mix( func_args ) {
exact_args( 5 )
    ModelContainer *src = nullptr, *other = nullptr, *dest = nullptr;

    dest = VNAssets::get_model( args[0].value_string() );
    src = VNAssets::get_model( args[1].value_string() );
    other = VNAssets::get_model( args[2].value_string() );
    if(!src){
        VNDebug::runtime_error("Source model not defined",args[0].value_string(),vni);
        return;
    }
    if(!other){
        VNDebug::runtime_error("Other model not defined",args[0].value_string(),vni);
        return;
    }
    if(!dest){
        VNDebug::runtime_error("Destination model not defined",args[0].value_string(),vni);
        return;
    }

    uint8_t attrb = ATTRB_POS;
    if(args[3].value_string() == "pos"){
        attrb = ATTRB_POS;
    }
    if(args[3].value_string() == "norm"){
        attrb = ATTRB_NORM;
    }
    if(args[3].value_string() == "color"){
        attrb = ATTRB_COL;
    }
    if(args[3].value_string() == "uv"){
        attrb = ATTRB_UV;
    }

    dest->mesh.set_from_blend(src->mesh, other->mesh, attrb, args[4].value_float());
}

// model copy <src> <dest>
void VNOP::model_copy( func_args ) {
    exact_args( 2 )
    ModelContainer *src = nullptr, *dest = nullptr;
    src = VNAssets::get_model( args[0].value_string() );
    src = VNAssets::get_model( args[1].value_string() );
    if(!src){
        VNDebug::runtime_error("Source model not defined",args[0].value_string(),vni);
        return;
    }

    if(!dest){
        VNDebug::runtime_error("Destination model not defined",args[1].value_string(),vni);
        return;
    }

    dest->mesh.clear();
    dest->mesh.append_mesh( src->mesh );
}

// model shader <name> <shader>
void VNOP::model_shader( func_args ) {
    exact_args( 2 )
    VNAssets::link_model_to_shader( args[0].value_string(), args[1].value_string() );
}
