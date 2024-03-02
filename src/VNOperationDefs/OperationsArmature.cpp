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
        format_map[armature_softbody]={"armature softbody -object -joint ( elasticity drag friction rigidity gravity joint-length ) -value"};

        operation_map["armature playing"] = armature_playing;
        format_map[armature_playing]={"armature playing -object -animation -time"};

        operation_map["armature play"] = armature_play;
        format_map[armature_play]={"armature play -object -animation | -speed ( loop back-forth end )"};

        operation_map["armature stop"] = armature_stop;
        format_map[armature_stop]={"armature stop -object | animations..."};

        operation_map["armature stop-all"] = armature_stop_all;
        format_map[armature_stop_all]={"armature stop-all -object"};

        operation_map["armature pose"] = armature_pose;
        format_map[armature_pose]={"armature pose -object -animation -time"};

        /*
         *NOTE armatures need rewritten
         * Currently armatures layer animations by play order and do not reset when done
         * The last played animation overrides all others, only the saved joints are written (unless exported otherwise)
         * Animations should support blending like add, weighted, or override which changes how they are applied
         * Posing should be directly integrated as well allowing for poses to be made between animations
         * Keyframes should follow the Keyframe type defined in the VNAssetManager allowing for fun interpolation types
         */

    }
};

void VNOP::armature_create(func_args){
}

void VNOP::armature_playing( func_args ) {
}

void VNOP::armature_play( func_args ) {
}

void VNOP::armature_stop( func_args ) {
}

void VNOP::armature_stop_all(func_args){
}

void VNOP::armature_pose( func_args ) {
}

void VNOP::armature_load( func_args ) {
}

void VNOP::armature_softbody( func_args ) {
}
