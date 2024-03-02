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
    void load_ops_image();
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
    math_add,
    math_sub,
    math_mul,
    math_div,
    math_mod,
    math_pow,
    math_abs,
    math_min,
    math_max,
    math_greater,
    math_lesser,
    math_invert,
    math_ceil,
    math_floor,
    math_interval,
    math_clamp,

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

    // logic
    logic_equals,
    logic_negate,
    logic_and,
    logic_or,
    logic_xor,

    // armature
    armature_load,
    armature_create,
    armature_softbody,
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

    // character
    character_create,
    character_name,
    say_element,
    say_start,
    say_continue,
    say_finish,

    // control
    routine,
    jump,
    jumpto,
    jump_return,
    branch,
    ifbranch,
    allow,
    block,
    match,
    option,
    lock_match,
    end_match,
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

    // image
    image_create,
    image_load,

    // modle
    model_create,
    model_clear,
    model_load,
    model_append,
    model_mix,
    model_copy,
    model_shader,

    // object
    object_create,
    object_model,
    object_image,
    object_set,
    object_get,
    object_translate,
    object_rotate,
    object_scale,
    object_animate,

    // shader
    shader_create,
    shader_load,
    shader_image,

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

    // view
    view_get,
    view_set,
    view_animate;
};


/*
     *  Place the string formats, the raw formats can be processed later from the string format.
     *  keyword
     *  <variable>
     *  <variable-list>{min}
     *  <variable-list>{min,max}
     *  &reference
     *  (enum1|enum2|enum3|enum4)
     */
        /*

        //audio
        {audio_create,          {"audio create <name>"}},
        {audio_delete,          {"audio delete <name>"}},
        {audio_play,            {"audio play <name>"}},
        {audio_pause,           {"audio pause <name>"}},
        {audio_pitch,           {"audio pitch <name> <pitch>"}},
        {audio_position,        {"audio position <name> <position>"}},

        // gui
        {gui_menu_create,       {"menu create <name>"}},
        {gui_menu_activate,     {"menu activate <name>"}},
        {gui_menu_compact,      {"menu compact (left|right) <element-offset> <pos1> <pos2> <top> <space> <line-space>"}},
        {gui_element_create,    {"element create <name> (text|button|toggle|bar|num-input|text-input|key-capture)"}},
        {gui_element_delete,    {"element delete <name>"}},
        {gui_element_enable,    {"element enable <name> <bool>"}},
        {gui_element_value,     {"element value (get|set) <name> &value"}},
        {gui_element_text,      {"element text (get|set) <name> &text"}},
        {gui_element_size,      {"element size (get|set) <name> &dimensions"}},
        {gui_element_position,  {"element position (get|set) <name> &position"}},
        {gui_element_routine,   {"element routine <name> <file> <label>"}},

        // model
        {model_create,          {"model create <name>"}},
        {model_clear,           {"model clear <name>"}},
        {model_load,            {"model load <name> <file>"}},
        {model_append,          {"model append <name> <source>"}},
        {model_mix,             {"model mix <name> <from> <to> (pos|norm|uv|color) <factor>"}},
        {model_copy,            {"model copy <name> <source>"}},
        {model_shader,          {"model shader <name> <shader>"}},

        // shader,
        {shader_create,         {"shader create <name>"}},
        {shader_load,           {"shader load <name>"}},
        {shader_image,          {"shader image <name> <image>"}},

        // view
        {view_position,         {"view position (get|set) &position"}},
        {view_focus,            {"view focus (get|set) &focus"}},
        {view_fov,              {"view fov (get|set) &fov"}},
        {view_animate,          {"view animate (position|focus|fov) <value> <time> (linear|power|ease|spring|bounce) <modifier>"}},

        // object,
        {object_create,         {"object create <name>"}},
        {object_model,          {"object model <name> <model>"}},
        {object_image,          {"object image <name> <image>"}},
        {object_set,            {"object (set|get) (position|rotation|scale) <name> <value> "}},
        {object_get,            {"object "}},
        {object_translate,      {"object "}},
        {object_rotate,         {"object "}},
        {object_scale,          {"object "}},
        {object_animate,        {"object "}};

        // armature,
        armature_load,
        armature_softbody,
        action_playing,
        action_play,
        action_pause,
        action_stop,
        action_stop_all,
        action_modify,
        action_pose,

        // image,
        image_create,
        image_load,

        // character,
        character_create,
        character_property,
        character_armature,
        character_name,

        say_start,
        say_continue,
        say_finish,

        // control flow
        routine,
        jump,
        jump_return,
        exit_interpreter,
        branch,
        branch_until,
        allow,
        block,
        match,
        option,
        done,
        end_match,
        wait,
        resume,

        // redefiniton
        def_var,
        cast_var,

        // math,
        math_add,
        math_sub,
        math_mul,
        math_div,
        math_mod,
        math_pow,
        math_abs,
        math_min,
        math_max,
        math_greater,
        math_lesser,
        math_invert,
        math_ceil,
        math_floor,
        math_interval,
        math_clamp,

        // vecmath,
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

        // quat,
        quat_from_axis,
        quat_rotate,
        quat_normalize,
        quat_nlerp,
        quat_rotate_vec,
        quat_mul,
        quat_from_vecs,
        quat_look,
        quat_identity,

        copy,
        at,

        // logic,
        logic_equals,
        logic_negate,
        logic_and,
        logic_or,
        logic_xor,

        // str,
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

        print;
        */

#endif // OPERATION_DEFS_H
