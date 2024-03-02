#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP {

    void load_ops_view() {
        operation_map["view get"] = view_get;
        format_map[view_get] = {"view get ( position rotation focus fov ) &dest"};

        operation_map["view set"] = view_set;
        format_map[view_set] = {"view set ( position rotation focus fov ) -value"};

        operation_map["view animate"] = view_animate;
        format_map[view_animate] = {"view animate ( position focus fov ) -value -time | ( linear power ease spring bounce ) -mod"};

    }
}

void VNOP::view_set(func_args){
    vec4 v;
    args[1].value_vec(v);
    if(args[0].value_string() == "position"){
        glm_vec3_copy(v, VNAssets::view.pos);
    }
    else if(args[0].value_string() == "rotation"){
        glm_quat_copy(v, VNAssets::view.rot);
    }
    else if(args[0].value_string() == "focus"){
        glm_vec3_copy(v, VNAssets::focus);
    }
    else if(args[0].value_string() == "fov"){
        VNAssets::view.setFOV(v[0]);
    }
}

void VNOP::view_get(func_args){
    vec4 v = GLM_VEC4_ZERO_INIT;
    args[1].value_vec(v);
    if(args[0].value_string() == "position"){
        glm_vec3_copy(VNAssets::view.pos,v);
    }
    else if(args[0].value_string() == "rotation"){
        glm_quat_copy(VNAssets::view.rot, v);
    }
    else if(args[0].value_string() == "focus"){
        glm_vec3_copy( VNAssets::focus, v);
    }
    else if(args[0].value_string() == "fov"){
        v[0] = VNAssets::view.getFOV();
    }
    VNI::variables.at(args[1].var_id).set(v);
}

// view animate <position rotation fov> <value> <length> [interp type] [mod]
void VNOP::view_animate( func_args ) {
    std::string type = args[0].value_string();
    uint8_t interp_type =  Interp::LINEAR;
    vec4 v;
    args[1].value_vec( v );
    float length = args[2].value_float();
    float mod = 2;

    // Optional interpolation values
    if( args.size() == 5 ) {
        std::string interp = args[3].value_string();

        if( interp == "linear" ) {
            interp_type = Interp::LINEAR;
        }
        else if( interp == "power" ) {
            interp_type = Interp::POWER;
        }
        else if( interp == "ease" ) {
            interp_type = Interp::EASE;
        }
        else if( interp == "spring" ) {
            interp_type = Interp::SPRING;
        }
        else if( interp == "bounce" ) {
            interp_type = Interp::BOUNCE;
        }

        mod = args[4].value_float();
    }

    if( type == "position" ) {
        glm_vec3_copy( VNAssets::view.pos, VNAssets::key_pos.from );
        glm_vec3_copy( v, VNAssets::key_pos.to );
        VNAssets::key_pos.t = 0;
        VNAssets::key_pos.type = interp_type;
        VNAssets::key_pos.length = length;
        VNAssets::key_pos.mod = mod;
    }
    else if( type == "focus" ) {
        glm_vec3_copy( VNAssets::focus, VNAssets::key_focus.from );
        glm_vec3_copy( v, VNAssets::key_focus.to );
        VNAssets::key_focus.t = 0;
        VNAssets::key_focus.type = interp_type;
        VNAssets::key_focus.length = length;
        VNAssets::key_focus.mod = mod;
    }
    else if( type == "fov" ) {
        VNAssets::key_fov.from = VNAssets::view.getFOV();
        VNAssets::key_fov.to = v[0];
        VNAssets::key_fov.t = 0;
        VNAssets::key_fov.type = interp_type;
        VNAssets::key_fov.length = length;
        VNAssets::key_fov.mod = mod;
    }

}
