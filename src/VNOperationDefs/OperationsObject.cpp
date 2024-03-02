#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP {

    void load_ops_object() {
        operation_map["object create"] = object_create;
        operation_map["object model"] = object_model;
        operation_map["object image"] = object_image;
        operation_map["object set"] = object_set;
        operation_map["object get"] = object_get;
        operation_map["object translate"] = object_translate;
        operation_map["object rotate"] = object_rotate;
        operation_map["object scale"] = object_scale;
        operation_map["object animate"] = object_animate;
    }
};

// object create <name>
void VNOP::object_create( func_args ) {
    exact_args( 1 )
    std::string name = args[0].value_string();
    VNAssets::create_object( name );
}

// object model <name> <model>
void VNOP::object_model( func_args ) {
    exact_args( 2 )
    VNAssets::link_object_to_model( args[0].value_string(), args[1].value_string() );
}

void VNOP::object_image( func_args ) {
    exact_args( 2 )
    VNAssets::link_object_to_image( args[0].value_string(), args[1].value_string() );
}

// object set <position/rotation/scale> <name> <value>
void VNOP::object_set( func_args ) {
    exact_args( 3 )
    ObjectInstance *obj = nullptr;
    obj = VNAssets::get_object(args[1].value_string());

    if( !obj )
        return;

    if( args[0].value_string() == "position" ) {
        args[2].value_vec3( obj->position );
    }
    else if( args[0].value_string() == "rotation" ) {
        args[2].value_vec( obj->rotation );
    }
    else if( args[0].value_string() == "scale" ) {
        obj->scale = args[2].value_float();
    }
}

// TODO
void VNOP::object_get( func_args ){}

// object translate <name> <var>
void VNOP::object_translate( func_args ) {
    exact_args( 2 )
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());

    if( !obj )
        return;

    vec4 v;
    args[1].value_vec( v );
    glm_vec3_add( obj->position, v, obj->position );
}

// object rotate <name> <quat or axis> <var>
void VNOP::object_rotate( func_args ) {
    exact_args( 3 )
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());

    if( !obj )
        return;

    std::string type = args[1].value_string();
    vec4 v;
    args[2].value_vec( v );

    if( type == "axis" ) {
        glm_quat( obj->rotation, v[3], v[0], v[1], v[2] );
    }
    else if( type == "quat" ) {
        glm_quat_mul( obj->rotation, v, obj->rotation );
    }
}

// object scale <name> <var>
void VNOP::object_scale( func_args ) {
    exact_args( 2 )
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());

    if( !obj )
        return;

    float s = args[1].value_float();
    obj->scale *= s;
}

// object animate <position/rotation/scale> <name>  <value> <length> <interp_type> <interp_value>
void VNOP::object_animate( func_args ) {
    ensure_args( 4 )
    ObjectInstance *obj = VNAssets::get_object(args[1].value_string());

    if( !obj )
        return;

    std::string type = args[0].value_string();
    uint8_t interp_type =  Interp::LINEAR;
    vec4 v;
    args[2].value_vec( v );
    float length = args[3].value_float();
    float mod = 2;

    // Optional interpolation values
    if( args.size() == 6 ) {
        std::string interp = args[4].value_string();

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

        mod = args[5].value_float();
    }

    if( type == "position" ) {
        glm_vec3_copy( obj->position, obj->key_position.from );
        glm_vec3_copy( v, obj->key_position.to );
        obj->key_position.t = 0;
        obj->key_position.type = interp_type;
        obj->key_position.length = length;
        obj->key_position.mod = mod;
    }
    else if( type == "rotation" ) {
        glm_quat_copy( obj->rotation, obj->key_rotation.from );
        glm_quat_copy( v, obj->key_rotation.to );
        obj->key_rotation.t = 0;
        obj->key_rotation.type = interp_type;
        obj->key_rotation.length = length;
        obj->key_rotation.mod = mod;
    }
    else if( type == "scale" ) {
        obj->key_scale.from = obj->scale;
        obj->key_scale.to = v[0];
        obj->key_scale.t = 0;
        obj->key_scale.type = interp_type;
        obj->key_scale.length = length;
        obj->key_scale.mod = mod;
    }

}
