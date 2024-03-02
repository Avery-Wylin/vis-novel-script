#ifndef VNVARIABLE_H
#define VNVARIABLE_H

#include <string>

#include "definitions.h"
#include "cglm/vec4.h"
#include <unordered_map>
#include <vector>
#include <regex>

static const uint8_t
VAR_FLOAT = 0,
VAR_INT = 1,
VAR_BOOL = 2,
VAR_VEC = 3,
VAR_STRING = 4;

// Regex definitions for matching valid variables
static std::regex

// vec accepts both floats and ints separated by spaces and eclosed in {}
regex_vec = std::regex( R"(\{\s+(?:\-?(?:(([0-9]+)?(?:\.[0-9]+))|(?:[0-9]+))\s+){1,4}\})" ),

// floats must have a .
regex_float = std::regex( R"(\-?(?:[0-9]+)?(?:\.[0-9]+))" ),

// ints are any number without an .
regex_int = std::regex( R"(\-?[0-9]+)" ),

// a variable name must start with a letter and only contain alphanumerics, _
regex_var_name = std::regex( R"([a-zA-Z]+[a-zA-Z0-9_]*)" ),

// Vectors can read in colors matching the color codes
// Color codes as strings translate to vectors and require a function to re-extract
regex_vec_color = std::regex(R"(#[a-fA-F0-9]{6})");

/*
 * There are 5 variable types:
 * bool
 * int
 * float
 * string
 * vec
 *
 * They must all be interchangeable with one another.
 */

class VNVariable {

        std::string stringval = "";
        uint8_t type = 0;
        uint8_t valid_flags = 0;    // Valid flags are stored in order of each type enum
        bool boolval = false;
        bool locked = false;
        vec4 floatvals = GLM_VEC4_ZERO_INIT;
        int intval = 0;

        inline bool is_valid( uint8_t t ) {
            return valid_flags & ( 1 << t );
        }

        inline void write_valid( uint8_t t ) {
            valid_flags |= ( 1 << t );
        }

        inline void write_invalid( uint8_t t ) {
            valid_flags &= ~( 1 << t );
        }

        void update_float();
        void update_int();
        void update_bool();
        void update_string();
        void update_vec();
        void update_type(uint8_t t);

    public:
        uint32_t var_id = 0;

        // Cast is the same as a copy followed by a convert
        void cast( uint8_t t );
        void string_infer_cast();
        void copy(VNVariable &dest);

        void clear();
        bool empty() const;

        void set( float f );
        void set( int i );
        void set( bool b );
        void set( vec4 v );
        void set( const std::string &s );

        VNVariable(){};
        VNVariable( float f ){cast(VAR_FLOAT);set(f);};
        VNVariable( int i ){cast(VAR_INT);set(i);};
        VNVariable( bool b ){cast(VAR_BOOL);set(b);};
        VNVariable( vec4 v ){cast(VAR_VEC);set(&v[0]);};
        VNVariable( const std::string &s ){cast(VAR_STRING);set(s);};

        float *glm_vec() const {
            return const_cast<float *>( floatvals );
        }

        bool equals( VNVariable &other );

        inline uint8_t get_type() const { return type;}

        inline void lock() {
            locked = true;
        }

        // When getting a value, it may be cast if the write is invalid
        float value_float();
        int value_int();
        bool value_bool();
        void value_vec( vec4 dest );
        void value_vec3( vec3 dest );
        const std::string &value_string();
};

class VNVariableContainer {
        std::unordered_map<std::string, uint32_t> vtable;
        std::vector<VNVariable> variables;
    public:

        VNVariableContainer() {
            // The table may return 0, which means an empty default variable should be created
            variables.push_back( VNVariable() );
        }

        bool contains( std::string &name ) {
            return vtable.contains( name );
        };

        void place( std::string &name, VNVariable &v ) {

            if( vtable[name] != 0 ) {
                variables[vtable[name]] = v;
                return;
            }

            vtable[name] = variables.size();
            variables.push_back( v );
            return;
        }

        inline VNVariable &at( uint32_t i ) {
            if( i >= variables.size() )
                return variables[0];

            return variables[i];
        }

        inline uint32_t get_id( std::string name ){
            try{
                return vtable.at(name);
            }
            catch(std::out_of_range &oor){
                return 0;
            }
        }

        inline void clear() {
            vtable.clear();
            variables.clear();
        }

};

#endif // VNVARIABLE_H
