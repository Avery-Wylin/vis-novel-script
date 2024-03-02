#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP {

    void load_ops_shader() {
        operation_map["shader create"] = shader_create;
        operation_map["shader load"] = shader_load;
        operation_map["shader image"] = shader_image;
    }
};

// shader create <name>
void VNOP::shader_create( func_args ) {
    exact_args( 1 )
    VNAssets::create_shader( args[0].value_string() );
}

// shader load <name> <filename>
void VNOP::shader_load( func_args ) {

    exact_args( 2 )
    ShaderContainer *s = VNAssets::get_shader( args[0].value_string() );

    if( !s )
        return;

    // This must be done on the thread containing the GL Context
    s->filename = args[1].value_string();
    s->needs_compiled = true;
}

// TODO
void VNOP::shader_image( func_args ) {}
