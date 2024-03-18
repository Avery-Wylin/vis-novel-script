#include "VNInterpreter.h"
#include "OperationDefs.h"
#include "VNAssetManager.h"

namespace VNOP{

    void load_ops_scene(){

        operation_map["scene select"] = scene_select;
        format_map[scene_select] = {"scene selecct -name"};

        operation_map["scene create"] = scene_create;
        format_map[scene_create] = {"scene create -name"};

        operation_map["scene delete"] = scene_delete;
        format_map[scene_delete] = {"scene delete -name"};

        operation_map["scene add"] = scene_add;
        format_map[scene_add] = {"scene add -object"};

        operation_map["scene remove"] = scene_remove;
        format_map[scene_remove] = {"scene remove -object"};

    }
};

void VNOP::scene_create(func_args){
    VNAssets::scene_create(args[0].value_string());
}

void VNOP::scene_delete(func_args){
    VNAssets::scene_delete(args[0].value_string());
}

void VNOP::scene_select(func_args) {
    VNAssets::scene_select(args[0].value_string());
}

void VNOP::scene_add(func_args){
    VNAssets::scene_add(args[0].value_string());
}

void VNOP::scene_remove(func_args){
    VNAssets::scene_remove(args[0].value_string());
}


