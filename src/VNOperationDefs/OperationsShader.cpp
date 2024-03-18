#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP {

    void load_ops_shader() {
        operation_map["shader create"] = shader_create;
        format_map[shader_create] = {"shader create -name"};

        operation_map["shader setting"] = shader_setting;
        format_map[shader_setting] = {"shader setting -name ( cull blend depth ) -v"};


        operation_map["shader load"] = shader_load;
        format_map[shader_load] = {"shader load -name -filename"};

    }
};

// shader create <name>
void VNOP::shader_create( func_args ) {
    ShaderContainer* s = VNAssets::create_shader( args[0].value_string() );
    s->cull = args[1].value_bool();
    s->blend = args[2].value_bool();
    s->depth_test = args[3].value_bool();
}


void VNOP::shader_setting(func_args){
    ShaderContainer *s = VNAssets::get_shader( args[0].value_string() );
    if( !s )
        return;
    switch(args[1].value_string().front()){
        case 'c': s->cull = args[2].value_bool(); break;
        case 'b': s->blend = args[2].value_bool(); break;
        case 'd': s->depth_test = args[2].value_bool(); break;
    }
}


// shader load <name> <filename>
void VNOP::shader_load( func_args ) {
    ShaderContainer *s = VNAssets::get_shader( args[0].value_string() );
    if( !s )
        return;

    // This must be done on the thread containing the GL Context
    s->filename = args[1].value_string();
    s->needs_compiled = true;
}

