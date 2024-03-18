#include "VNInterpreter.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"

namespace VNOP {
    void load_ops_armature() {
        operation_map["armature load"] =  armature_load ;
        format_map[armature_load] = {"armature load -filename"};

        operation_map["armature create"] = armature_create;
        format_map[armature_create] = {"armature create -object -armature"};

        operation_map["armature softbody"] =  armature_softbody ;
        format_map[armature_softbody]={"armature softbody -object -joint ( elasticity drag friction rigidity gravity length all ) -value | values... "};

        operation_map["armature track"] = armature_track;
        format_map[armature_track] = {"armature track -object -joint -focus ( +x +y +z -x -y -z )"};

        operation_map["armature playing"] = armature_playing;
        format_map[armature_playing]={"armature playing -object -animation -time"};

        operation_map["armature play"] = armature_play;
        format_map[armature_play]={"armature play -object -animation | -speed ( loop back pingpong end clamp ) ( wait update overwrite ) ( add mix set ) -mix -range"};

        operation_map["armature stop"] = armature_stop;
        format_map[armature_stop]={"armature stop -object | animations..."};

        operation_map["armature stop-all"] = armature_stop_all;
        format_map[armature_stop_all]={"armature stop-all -object"};

        operation_map["armature pose"] = armature_pose;
        format_map[armature_pose]={"armature pose -object -animation -time"};
    }
};

void VNOP::armature_create(func_args){
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());
    ArmatureInfo *info = VNAssets::get_armature_info(args[1].value_string());
    if(!obj){
        VNDebug::runtime_error("Object not found",args[0].value_string(), vni);
        return;
    }
    if(!info){
        VNDebug::runtime_error("Armature info not found",args[1].value_string(), vni);
        return;
    }
    obj->armature.assign_info(info);
}

void VNOP::armature_playing( func_args ) {
}

// armature play -object -animation | -speed ( loop back pingpong end clamp ) ( wait update overwrite ) ( add mix set ) -mix -range
void VNOP::armature_play( func_args ) {
     ObjectInstance *obj = VNAssets::get_object(args[0].value_string());
    if(!obj){
        VNDebug::runtime_error("Object not found",args[0].value_string(), vni);
        return;
    }
    if(obj->armature.empty()){
        VNDebug::runtime_error("Object does not have an armature",args[0].value_string(), vni);
        return;
    }
    uint8_t anim = obj->armature.get_info()->get_animation_id(args[1].value_string());
    if(!anim){
        VNDebug::runtime_error("Animation not found",args[1].value_string(), vni);
        return;
    }

    float speed = 1;
    vec4 range = {0, FLT_MAX, 0, 0};
    uint8_t end_behavior = PlayData::END,
    mix_behavior = PlayData::SET,
    write_behavior = PlayData::OVERWRITE;
    float mix = .5;

    if(args.size() > 2)
        speed = args[2].value_float();
    if(args.size() > 3){
        switch(args[3].value_string().front()){
            case 'e': end_behavior = PlayData::END; break;
            case 'l': end_behavior = PlayData::LOOP; break;
            case 'b': end_behavior = PlayData::BACK; break;
            case 'p': end_behavior = PlayData::BACK_LOOPED; break;
            case 'c': end_behavior = PlayData::CLAMPED; break;
        }
    }
    if(args.size() > 4){
        switch(args[4].value_string().front()){
            case 'o': write_behavior = PlayData::OVERWRITE; break;
            case 'w': write_behavior = PlayData::WAIT; break;
            case 'u': write_behavior = PlayData::UPDATE; break;
        }
    }
    if(args.size() > 5){
        switch(args[5].value_string().front()){
            case 's': mix_behavior = PlayData::SET; break;
            case 'm': mix_behavior = PlayData::MIX; break;
            case 'a': mix_behavior = PlayData::ADD; break;
        }
    }
    if(args.size() > 6){
        mix = args[6].value_float();
    }
    if(args.size() > 7){
        args[7].value_vec(range);
    }


    obj->armature.play_animation(anim, speed, range[0], range[1], end_behavior, mix_behavior, mix, write_behavior);
}

void VNOP::armature_stop( func_args ) {
}

void VNOP::armature_stop_all(func_args){
}

void VNOP::armature_pose( func_args ) {
     ObjectInstance *obj = VNAssets::get_object(args[0].value_string());
    if(!obj){
        VNDebug::runtime_error("Object not found",args[0].value_string(), vni);
        return;
    }
    if(obj->armature.empty()){
        VNDebug::runtime_error("Object does not have an armature",args[0].value_string(), vni);
        return;
    }
    uint8_t anim = obj->armature.get_info()->get_animation_id(args[1].value_string());
    if(!anim){
        VNDebug::runtime_error("Animation not found",args[1].value_string(), vni);
        return;
    }

    obj->armature.pose(anim, args[2].value_float() , 1);
}

void VNOP::armature_load( func_args ) {
    VNAssets::load_armature(args[0].value_string(), args[0].value_string());
}

// "armature softbody -object -joint ( elasticity drag friction rigidity gravity length all ) -value | values... "
void VNOP::armature_softbody( func_args ) {
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());
    uint8_t jid = 0;

    if(!obj){
        VNDebug::runtime_error("Object does not exist",args[0].value_string(), vni);
        return;
    }

    if(obj->armature.empty()){
        VNDebug::runtime_error("Object does not have an armature",args[0].value_string(), vni);
        return;
    }

    jid = obj->armature.get_info()->get_joint_id(args[1].value_string());
    if(jid == 0){
        VNDebug::runtime_error("Unknown joint",args[1].value_string(), vni);
        return;
    }

    Constraint *c = obj->armature.get_constraint( obj->armature.get_joint(jid).constraint_id);
    SoftbodySettings sb;
    if(c && c->type == Constraint::SOFTBODY){
        sb = ((ConstraintSoftbody*)c)->settings;
    }

    switch(args[2].value_string().front()){
        case 'e' : sb.elasticity = args[3].value_float(); break;
        case 'd' : sb.drag = args[3].value_float(); break;
        case 'f' : sb.friction = args[3].value_float(); break;
        case 'r' : sb.rigidity = args[3].value_float(); break;
        case 'g' : sb.gravity = args[3].value_float(); break;
        case 'l' : sb.joint_length = args[3].value_float(); break;

        case 'a' :
            if( args.size() != 8 ) {
                VNDebug::runtime_error( "Not enough args for 'all' expected 8, got", std::to_string( args.size() ), vni );
                return;
            }
            sb.elasticity = args[3].value_float();
            sb.drag = args[4].value_float();
            sb.friction = args[5].value_float();
            sb.rigidity = args[6].value_float();
            sb.gravity = args[7].value_float();
            break;
    }

    obj->armature.constraint_softbody(jid, sb);
}

void VNOP::armature_track(func_args){
    ObjectInstance *obj = VNAssets::get_object(args[0].value_string());
    uint8_t jid = 0;

    if(!obj){
        VNDebug::runtime_error("Object does not exist",args[0].value_string(), vni);
        return;
    }

    if(obj->armature.empty()){
        VNDebug::runtime_error("Object does not have an armature",args[0].value_string(), vni);
        return;
    }

    jid = obj->armature.get_info()->get_joint_id(args[1].value_string());
    if(jid == 0){
        VNDebug::runtime_error("Unknown joint",args[1].value_string(), vni);
        return;
    }

    vec4 focus;
    args[2].value_vec(focus);

    bool n = args[3].value_string()[0] == '-';
    // Take advantage of the xyz order in ASCII
    uint8_t axis = args[3].value_string()[1] - 'x';

    obj->armature.constraint_track(jid, focus, axis, n);
}
