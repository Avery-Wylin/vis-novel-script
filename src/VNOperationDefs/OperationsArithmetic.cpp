#include "VNInterpreter.h"
#include "GUI.h"
#include "VNAssetManager.h"
#include "OperationDefs.h"
#include "ExpressionParser.h"

namespace VNOP {

    void load_ops_arithmetic() {

        format_map[expr] = {"expr &dest | args..."};

        // math 1
        operation_map["add"] = math_add;
        format_map[math_add] = {"add -v1 -v2 &dest"};

        operation_map["sub"] = math_sub;
        format_map[math_sub] = {"sub -v1 -v2 &dest"};

        operation_map["mul"] = math_mul;
        format_map[math_mul] = {"mul -v1 -v2 &dest"};

        operation_map["div"] = math_div;
        format_map[math_div] = {"div -v1 -v2 &dest"};

        operation_map["mod"] = math_mod;
        format_map[math_mod] = {"mod -v1 -v2 &dest"};

        operation_map["pow"] = math_pow;
        format_map[math_pow] = {"pow -v1 -v2 &dest"};

        operation_map["abs"] = math_abs;
        format_map[math_abs] = {"abs -v1 -v2 &dest"};

        operation_map["min"] = math_min;
        format_map[math_min] = {"min -v1 -v2 &dest"};

        operation_map["max"] = math_max;
        format_map[math_max] = {"min -v1 -v2 &dest"};

        operation_map["greater"] = math_greater;
        format_map[math_greater] = {"min -v -threshold &bool"};

        operation_map["lesser"] = math_lesser;
        format_map[math_lesser] = {"lesser -v -threshold &bool"};

        operation_map["invert"] = math_invert;
        format_map[math_invert] = {"invert -v &dest"};

        operation_map["ceil"] = math_ceil;
        format_map[math_ceil] = {"ceil -v &dest"};

        operation_map["floor"] = math_floor;
        format_map[math_floor] = {"floor -v &dest"};

        operation_map["interval"] = math_interval;
        format_map[math_interval] = {"interval -v -fsnap &dest"};

        operation_map["clamp"] = math_clamp;
        format_map[math_clamp] = {"clamp -v -min -max &dest"};

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

        // logic 1
        operation_map["equals"] =  logic_equals;
        format_map[logic_equals] = {"equals -v1 -v2_casted &bool"};

        operation_map["negate"] =  logic_negate;
        format_map[logic_negate] = {"negate -bool &!bool"};

        operation_map["and"] =  logic_and;
        format_map[logic_and] = {"and -b1 -b2 &bool"};

        operation_map["or"] =  logic_or;
        format_map[logic_or] = {"or -b1 -b2 &bool"};

        operation_map["xor"] =  logic_xor;
        format_map[logic_xor] = {"xor -b1 -b2 &bool"};

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

void VNOP::expr(func_args){
    VNI::variables.at(args[0].var_id).set(ExpressionParser::run_expression(1, args, vni));
}

// cast &var -type
void VNOP::cast_var( func_args ) {
    ensure_args( 2 )
    uint32_t vid = VNI::variables.get_id( args[0].value_string() );

    if( vid == 0 )
        return;
    else if( args[1].value_string() == "float" )
        args[1].cast( VAR_FLOAT );
    if( args[1].value_string() == "int" )
        args[1].cast( VAR_INT );
    else if( args[1].value_string() == "bool" )
        args[1].cast( VAR_BOOL );
    else if( args[1].value_string() == "vec" )
        args[1].cast( VAR_VEC );
    else if( args[1].value_string() == "string" )
        args[1].cast( VAR_STRING );
}

// Math

// Math-Op <src> <other> <dest>
void VNOP::math_add( func_args ) {
    arithmetic_src_other_dest( glm_vec4_add )
}
void VNOP::math_sub( func_args ) {
    arithmetic_src_other_dest( glm_vec4_sub );
}
void VNOP::math_mul( func_args ) {
    arithmetic_src_other_dest( glm_vec4_mul )
}
void VNOP::math_div( func_args ) {
    arithmetic_src_other_dest( glm_vec4_div )
}
void VNOP::math_mod( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 s, o, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    args[1].value_vec( o );
    d[0] = fmod( s[0], o[0] );
    d[1] = fmod( s[1], o[1] );
    d[2] = fmod( s[2], o[2] );
    d[3] = fmod( s[3], o[3] );
    dest.set( &d[0] );
}
void VNOP::math_pow( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 s, o, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    args[1].value_vec( o );
    d[0] = pow( s[0], o[0] );
    d[1] = pow( s[1], o[1] );
    d[2] = pow( s[2], o[2] );
    d[3] = pow( s[3], o[3] );
    dest.set( &d[0] );
}
void VNOP::math_min( func_args ) {
    arithmetic_src_other_dest( glm_vec4_minv )
}
void VNOP::math_max( func_args ) {
    arithmetic_src_other_dest( glm_vec4_maxv )
}
void VNOP::math_greater( func_args ) {

    vec4 s, o, d = GLM_VEC4_ZERO_INIT;
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    args[0].value_vec( s );
    args[1].value_vec( o );
    d[0] = s[0] > o[0];
    d[1] = s[1] > o[1];
    d[2] = s[2] > o[2];
    d[3] = s[3] > o[3];
    dest.set( &d[0] );
}
void VNOP::math_lesser( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 s, o, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    args[1].value_vec( o );
    d[0] = s[0] < o[0];
    d[1] = s[1] < o[1];
    d[2] = s[2] < o[2];
    d[3] = s[3] < o[3];
    dest.set( &d[0] );
}

void VNOP::math_interval( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    vec4 s, o, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    args[1].value_vec( o );
    if( glm_vec4_eq( o, 0 ) )
        return;
    d[0] = floor( s[0] * o[0] ) / o[0];
    d[1] = floor( s[1] * o[1] ) / o[1];
    d[2] = floor( s[2] * o[2] ) / o[2];
    d[3] = floor( s[3] * o[2] ) / o[3];
    dest.set( &d[0] );
}


// Math-Op <src> <dest>
void VNOP::math_abs( func_args ) {
    arithmetic_src_dest( glm_vec4_abs )
}
void VNOP::math_invert( func_args ) {
    arithmetic_src_dest( glm_vec4_inv_to )
}
void VNOP::math_ceil( func_args ) {
    VNVariable &dest = VNI::variables.at(args[1].var_id);
    vec4 s, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    d[0] = ceil( s[0] );
    d[1] = ceil( s[1] );
    d[2] = ceil( s[2] );
    d[3] = ceil( s[3] );
    dest.set( &d[0] );
}
void VNOP::math_floor( func_args ) {
    VNVariable &dest = VNI::variables.at(args[1].var_id);
    vec4 s, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    d[0] = floor( s[0] );
    d[1] = floor( s[1] );
    d[2] = floor( s[2] );
    d[3] = floor( s[3] );
    dest.set( &d[0] );
}


// Math-Op -v -min -max &dest
void VNOP::math_clamp( func_args ) {
    VNVariable &dest = VNI::variables.at(args[3].var_id);
    vec4 s, min, max, d = GLM_VEC4_ZERO_INIT;
    args[0].value_vec( s );
    args[1].value_vec( min );
    args[2].value_vec( max );
    d[0] = glm_clamp( s[0], min[0], max[0] );
    d[1] = glm_clamp( s[1], min[1], max[1] );
    d[2] = glm_clamp( s[2], min[2], max[2] );
    d[3] = glm_clamp( s[3], min[3], max[3] );
    dest.set( &d[0] );
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


// logic-op -v1 -v2 &dest
void VNOP::logic_equals( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    dest.set( !args[0].value_bool() == !args[1].value_bool() );
}
void VNOP::logic_and( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    dest.set( !args[0].value_bool() && !args[1].value_bool() );
}
void VNOP::logic_or( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    dest.set( !args[0].value_bool() || !args[1].value_bool() );
}
void VNOP::logic_xor( func_args ) {
    VNVariable &dest = VNI::variables.at(args[2].var_id);
    dest.set( !args[0].value_bool() || !args[1].value_bool() );
}

// negate -v1 &dest
void VNOP::logic_negate( func_args ) {
    VNVariable &dest = VNI::variables.at(args[1].var_id);
    dest.set( !args[0].value_bool() );
}

