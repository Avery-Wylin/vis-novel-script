#ifndef OPERATION_DEFS_H
#define OPERATION_DEFS_H

#include <unordered_map>
#include <string>
#include <vector>
#include "VNVariable.h"

// Forward Declarations
struct VNOperation;
class VNInterpreter;
class VNCompiledFile;

// NOTE these need to be removed and replaced with format checkers
#define ensure_args(count) \
    if(args.size() < count){\
    VNDebug::runtime_error( "Not enough arguments, expected ",std::to_string(count), vni );\
    return;}

#define exact_args(count) \
    if(args.size() != count){\
    VNDebug::runtime_error( "Exactly",std::to_string(count) + " args expected", vni);\
    return;}

#define dest_last \
if(args.back().var_id == 0 ) return;\
VNVariable &dest = VNI::variables.at(args.back().var_id);

// The arguments used for an operation function
#define func_args std::vector<VNVariable> &args, VNInterpreter &vni

// Types for the functions
typedef void ( OpFunc )( func_args );
typedef void ( *OpFuncPtr )( func_args );

// Format for an operation
struct OpFormatArg {
    static const uint8_t VAR = 0, ENUM = 1, REF = 2;
    uint8_t type = VAR;
    std::vector<std::string> enums;
};

struct OpFormat {
    const char* format_string = nullptr;
    // The minimum and maximum number of args to allow on compilation
    uint8_t min_args = 0, max_args = 0;
    bool reads_multiple_lines = false;
    std::vector<OpFormatArg> format;
    void init();
    bool validate(VNOperation &op, VNCompiledFile &vncf);
    void clear();
};

namespace VNOP {
    extern std::unordered_map< std::string, OpFuncPtr> operation_map;
    extern std::unordered_map< OpFuncPtr, OpFormat> format_map;

    // Each function loads a module by placing its keywords and format into the maps
    void load_ops_arithmetic();
    void load_ops_armature();
    void load_ops_audio();
    void load_ops_character();
    void load_ops_control();
    void load_ops_gui();
    void load_ops_scene();
    void load_ops_model();
    void load_ops_object();
    void load_ops_shader();
    void load_ops_string();
    void load_ops_view();
    void load_ops();

    // Use the operation format strings to fill the remaining operation format fields
    void init_op_formats();

    // Function Definitions

    OpFunc

    // math
    expr,

    // vecmath
    math_combine,
    math_separate,
    math_fill,
    math_sum,
    math_cross,
    math_project,
    math_distance,
    math_length,
    math_normalize,
    math_dot,
    math_lerp,

    // quat
    quat_from_axis,
    quat_rotate,
    quat_normalize,
    quat_nlerp,
    quat_rotate_vec,
    quat_mul,
    quat_from_vecs,
    quat_look,
    quat_identity,

    cast_var,
    copy,
    at,

    // armature
    armature_load,
    armature_create,
    armature_softbody,
    armature_track,
    armature_playing,
    armature_play,
    armature_stop,
    armature_stop_all,
    armature_pose,

    // audio
    sound,
    music_play,
    music_pause,
    music_stop,
    music_volume,
    music_pitch,

    // dialogue
    say_element,
    say_start,
    say_continue,
    say_print,

    // control
    routine,
    alias,
    jump,
    jumpto,
    jump_return,
    branch,
    ifbranch,
    exit_interpreter,
    wait,
    resume,
    def_var,
    print,

    // gui
    menu_create,
    menu_activate,
    menu_compact,
    element_create,
    element_delete,
    element_enable,
    element_value,
    element_text,
    element_size,
    element_position,
    element_routine,

    // scene
    scene_create,
    scene_delete,
    scene_select,
    scene_add,
    scene_remove,

    // model
    model_create,
    model_clear,
    model_load,
    model_append,
    model_mix,
    model_copy,
    model_images,

    // object
    object_create,
    object_model,
    object_shader,
    object_set,
    object_get,
    object_translate,
    object_rotate,
    object_scale,
    object_imgsel,
    object_animate,
    object_parent,

    // shader
    shader_create,
    shader_load,
    shader_setting,

    // string
    str_concat,
    str_clear,
    str_length,
    str_find,
    str_erase,
    str_trim,
    str_extract,
    str_matches,
    str_replace,
    str_compose,
    str_from,
    str_newline,

    // view
    view_get,
    view_set,
    view_animate;
};

#endif // OPERATION_DEFS_H
