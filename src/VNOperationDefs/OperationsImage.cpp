#include "VNInterpreter.h"
#include "OperationDefs.h"
#include "VNAssetManager.h"

namespace VNOP{

    void load_ops_image(){
        operation_map["image create"] = image_create;
        operation_map["image load"] = image_load;
    }
};

// image create <name>
void VNOP::image_create( func_args ) {
    exact_args( 1 )
    VNAssets::create_image( args[0].value_string() );
}

// image load <name> <filename>
void VNOP::image_load( func_args ) {
    exact_args( 2 )

    // Attempt to load an image container texture
    ImageContainer *img = VNAssets::get_image( args[0].value_string() );

    if( !img ){
        VNDebug::runtime_error("Image not defined", args[1].value_string(), vni);
        return;
    }

    // This must be done on the thread containing the GL Context
    img->filename = args[1].value_string();
    img->needs_loaded = true;
}
