#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP {

    void load_ops_object() {
        operation_map["object create"] = object_create;
        format_map[object_create] = {"object create -name"};

        operation_map["object model"] = object_model;
        format_map[object_model] = {"object model -name -model"};

        operation_map["object shader"] = object_shader;
        format_map[object_shader] = {"object shader -name -shader"};

        operation_map["object set"] = object_set;
        format_map[object_set] = {"object set ( position rotation scale ) -name -value"};

        operation_map["object get"] = object_get;
        format_map[object_get] = {"object get ( position rotation scale ) -name &value "};

        operation_map["object translate"] = object_translate;
        format_map[object_translate] = {"object translate -name -value"};

        operation_map["object rotate"] = object_rotate;
        format_map[object_rotate] = {"object rotate -name ( quat axis ) -value"};

        operation_map["object scale"] = object_scale;
        format_map[object_scale] = {"object scale -name -value"};

        operation_map["object animate"] = object_animate;
        format_map[object_animate] = {"object animate ( position rotation scale ) -name  -value -time | ( linear power ease spring bounce ) -mod"};

        operation_map["object imgsel"] = object_imgsel;
        format_map[object_imgsel] = {"object imgsel -name -img | -time"};

        operation_map["object parent"] =  object_parent;
        format_map[object_parent] = {"object parent -name | -parent -joint"};

    }
};

// object create <name>
void VNOP::object_create( func_args ) {
    std::string name = args[0].value_string();
    VNAssets::create_object( name );
}

// object model -name -model
void VNOP::object_model( func_args ) {
    ObjectInstance *obj = nullptr;
    obj = VNAssets::get_object(args[0].value_string());

    if(!obj)
        return;

    ModelContainer *model = nullptr;
    model = VNAssets::get_model(args[1].value_string());

    if(!model)
        return;

    obj->model = model - VNAssets::null_model();
}

// object shader -name -shader
void VNOP::object_shader( func_args ) {
    ObjectInstance *obj = nullptr;
    obj = VNAssets::get_object(args[0].value_string());

    if(!obj)
        return;

    ShaderContainer *shader = nullptr;
    shader = VNAssets::get_shader(args[1].value_string());

    if(!shader)
        return;

    obj->shader = shader - VNAssets::null_shader();
}


// object set <position/rotation/scale> <name> <value>
void VNOP::object_set( func_args ) {
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
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());

    if( !obj )
        return;

    vec4 v;
    args[1].value_vec( v );
    glm_vec3_add( obj->position, v, obj->position );
}

// object rotate <name> <quat or axis> <var>
void VNOP::object_rotate( func_args ) {
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
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());

    if( !obj )
        return;

    float s = args[1].value_float();
    obj->scale *= s;
}

// object animate <position/rotation/scale> <name>  <value> <length> <interp_type> <interp_value>
void VNOP::object_animate( func_args ) {
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

// object imgsel -name -img -time
void VNOP::object_imgsel(func_args){
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());

    if( !obj )
        return;

    if(!obj->model)
        return;

    ModelContainer *model = VNAssets::get_model(obj->model);

    // Move current texture id into old
    obj->tex_id[0] = obj->tex_id[1];

    // Load new image by either name or value
    if(args[1].get_type() == VAR_STRING){
        int32_t id = std::find(model->image_names.begin(), model->image_names.end(), args[1].value_string())-model->image_names.begin();
        if(id < 0){
            VNDebug::runtime_error("Unknown subimage", args[1].value_string(), vni);
            return;
        }
        else{
            obj->tex_id[1] = id;
        }
    }
    else
        obj->tex_id[1] = args[1].value_int();


    // Clamp the image range to that of the image image_array
    glm_vec3_clamp(obj->tex_id, 0, fmax(model->image_names.size()-1,0));

    if(args.size() == 2){
        obj->tex_id[0] = obj->tex_id[1];
        obj->key_texture_mix.length = 0;
        obj->key_texture_mix.t = 0;
    }

    // Set the time
    obj->key_texture_mix.from = 0;
    obj->key_texture_mix.to = 1;
    obj->key_texture_mix.t = 0;
    obj->key_texture_mix.length = args[2].value_float();
}

void VNOP::object_parent(func_args){
    // Remove parent
    if(args.size() == 1){
        // VNAssets::unparent(args[0].value_string());
    }
    // Parent to object
    else if(args.size() == 2){
        VNAssets::parent_object(args[0].value_string(), args[1].value_string());
    }
    // Parent to joint
    else if(args.size() == 3){
        VNAssets::parent_joint(args[0].value_string(), args[1].value_string(), args[2].value_string());
    }
}
