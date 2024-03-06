#include "VNInterpreter.h"
#include "GUI.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"
#include "ExpressionParser.h"

namespace VNOP {

    void load_ops_arithmetic() {

        format_map[expr] = {"expr &dest | args..."};

        // vecmath 1
        operation_map["combine"] =  math_combine;
        format_map[math_combine] = {"combine &vec | -x -y -z -w"};

        operation_map["separate"] =  math_separate;
        format_map[math_separate] = {"separate -src | &x &y &z &w"};

        operation_map["fill"] =  math_fill;
        format_map[math_fill] = {"fill &vec -value"};

        operation_map["sum"] =  math_sum;
        format_map[math_sum] = {"sum -vec &sum"};

        operation_map["cross"] =  math_cross;
        format_map[math_cross] = {"cross -v1 -v2 &vec"};

        operation_map["project"] =  math_project;
        format_map[math_project] = {"project -v1 -v2 &vec"};

        operation_map["distance"] =  math_distance;
        format_map[math_distance] = {"distance -v1 -v2 &dist"};

        operation_map["length"] =  math_length;
        format_map[math_length] = {"length -vec &len"};

        operation_map["normalize"] =  math_normalize;
        format_map[math_normalize] = {"normalize -vec &dest"};

        operation_map["dot"] =  math_dot;
        format_map[math_dot] = {"dot -v1 -v2 &dot"};

        operation_map["lerp"] =  math_lerp;
        format_map[math_lerp] = {"lerp -v1 -v2 -factor &dest"};


        // quatmath 2
        operation_map["quat from-axis"] = quat_from_axis;
        operation_map["quat rotate"] = quat_rotate;
        operation_map["quat normalize"] = quat_normalize;
        operation_map["quat nlerp"] = quat_nlerp;
        operation_map["quat rotate-vec"] = quat_rotate_vec;
        operation_map["quat mul"] = quat_mul;
        operation_map["quat from-vecs"] = quat_from_vecs;
        operation_map["quat look"] = quat_look;
        operation_map["quat identity"] = quat_identity;

        // converters 1
        operation_map["cast"] = cast_var;
        format_map[cast_var] = {"cast &var ( float int bool string vec )"};

        operation_map["copy"] = copy;
        format_map[copy] = {"copy -from &to"};

        operation_map["at"] = at;
        format_map[at] = {"at -vec -index &component"};
    }
}


#define arithmetic_src_other_dest( glm_vec_op )\
VNVariable &dest = VNI::variables.at(args[2].var_id);\
vec4 s,o,d;\
args[0].value_vec(s);\
args[1].value_vec(o);\
glm_vec_op(s,o,d);\
dest.set(d);

#define arithmetic_src_dest( glm_vec_op )\
VNVariable &dest = VNI::variables.at(args[1].var_id);\
vec4 s,d;\
args[0].value_vec(s);\
glm_vec_op(s,d);\
dest.set(d);


// This function now handles arithmetic and logic operations, it only works on floats
void VNOP::expr(func_args){
    VNI::variables.at(args[0].var_id).set(ExpressionParser::run_expression(1, args, vni));
}

// cast &var -type
void VNOP::cast_var( func_args ) {
    ensure_args( 2 )
    uint32_t vid = VNI::variables.get_id( args[0].value_string() );

    if( vid == 0 )
        return;
    switch(args[1].value_string()[0]){
        case 'f': args[1].cast( VAR_FLOAT ); break;
        case 'i': args[1].cast( VAR_INT ); break;
        case 'b': args[1].cast( VAR_BOOL ); break;
        case 'v': args[1].cast( VAR_VEC ); break;
        case 's': args[1].cast( VAR_STRING ); break;
    }
}

// vecmath

// combine <dest> <src 1-4>
void VNOP::math_combine( func_args ) {
    VNVariable &dest = VNI::variables.at(args[0].var_id);
    vec4 d = GLM_VEC4_ZERO_INIT;

    for( uint8_t i = 1; i < 5 && i < args.size(); ++i ) {
        d[i-1] = args[i].value_float();
    }
    dest.set( &d[0] );
}

// separate <src> <1-4 dest>
void VNOP::math_separate( func_args ) {
    vec4 v;
    args[0].value_vec( v );

    for( uint8_t i = 1; i < args.size() && i <= 4; ++i ) {
        if( args[i].var_id != 0 ) {
            VNVariable &dest = VNI::variables.at( args[i].var_id );
            uint8_t type = dest.get_type();
            dest.set( v[i - 1] );
            dest.cast(type);
        }
    }
}

// fill &dest -value
void VNOP::math_fill( func_args ) {
    VNVariable &dest = VNI::variables.at(args[0].var_id);
    vec4 d;
    glm_vec3_fill( d, args[1].value_float() );
    dest.set( &d[0] );
}

// sum -vec &sum
void VNOP::math_sum( func_args ) {
    VNVariable &dest = VNI::variables.at(args[1].var_id);
    vec4 s;
    args[0].value_vec( s );
    dest.set( glm_vec4_hadd( s ) );
}

// length -vec &length
void VNOP::math_length( func_args ) {
    VNVariable &dest = VNI::variables.at(args[1].var_id);
    vec4 s;
    args[0].value_vec( s );
    dest.set( glm_vec4_norm( s ) );
}

// normalize -vec &dest
void VNOP::math_normalize( func_args ) {
    VNVariable &dest = VNI::variables.at(args[1].var_id);
    vec4 d;
    args[0].value_vec( d );
    glm_vec4_normalize( d );
    dest.set( &d[0] );
    return;
}

// distance -v1 -v2 &dist
void VNOP::math_distance( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 s, o;
    args[0].value_vec( s );
    args[1].value_vec( o );
    dest.set( glm_vec4_distance( s, o ) );
}

// dot -v1 -v2 &dot
void VNOP::math_dot( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 s, o;
    args[0].value_vec( s );
    args[1].value_vec( o );
    dest.set( glm_vec4_dot( s, o ) );
}

// cross -v1 -v2 &cross
void VNOP::math_cross( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 s, o, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    args[1].value_vec( o );
    glm_vec3_cross( s, o, d );
    d[3] = 0;
    dest.set( &d[0] );
}

// project -v1 -v2 &proj
void VNOP::math_project( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 s, o, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    args[1].value_vec( o );
    glm_vec3_proj( s, o, d );
    d[3] = 0;
    dest.set( &d[0] );
}

// lerp -v1 -v2 -factor &dest
void VNOP::math_lerp( func_args ) {
    VNVariable &dest = VNI::variables.at(args[3].var_id);
    vec4 s, o, d;
    args[0].value_vec( s );
    args[1].value_vec( o );
    float f = args[2].value_float();
    glm_vec4_lerp( s, o, f, d );
    dest.set( &d[0] );
}

// quat nlerp <src> <other> <lerpval> <dest>
void VNOP::quat_nlerp( func_args ) {}

// quat from-axis <axis> <angle> <dest>
void VNOP::quat_from_axis( func_args ) {}

// quat rotate <src> <other> <dest>
void VNOP::quat_rotate( func_args ) {}

// quat normalize <dest>
void VNOP::quat_normalize( func_args ) {}

// quat rotate-vec <quat> <vec> <dest>
void VNOP::quat_rotate_vec( func_args ) {}

// NOTE this is not needed, use quat rotate
void VNOP::quat_mul( func_args ) {}

// quat from-vecs <from> <to> <dest>
void VNOP::quat_from_vecs( func_args ) {}

// quat look <direction> <up> <dest>
void VNOP::quat_look( func_args ) {}

// quat identity <dest>
void VNOP::quat_identity( func_args ) {}


// copy -from &to
void VNOP::copy( func_args ) {
    VNVariable &dest = VNI::variables.at(args[1].var_id);
    args[0].copy(dest);
}

// at -vec -index &component
void VNOP::at( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 v;
    args[0].value_vec(v);
    dest.set(v[abs(args[1].value_int())%4]);
}

