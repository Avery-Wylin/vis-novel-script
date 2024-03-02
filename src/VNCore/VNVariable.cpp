#include "VNVariable.h"
#include <cstring>
#include <stdexcept>
#include <cglm/vec3.h>


// Casting changes type but preserves value
void VNVariable::cast( uint8_t t ) {
    if(locked)
        return;

    // Check if the desired type is valid, if empty the type may also be changed
    if(empty() || is_valid(t)){
        // If it is valid, all that needs changed is the type
        type = t;
        return;
    }

    // If the type is not valid, mark all invalid except the old type then update
    valid_flags = 0;
    write_valid(type);
    update_type(t);
    type = t;
    return;
}

// Copies the current type to the variable
void VNVariable::copy( VNVariable &dest ){
    if(dest.locked)
        return;
    dest.valid_flags = 0;
    dest.write_valid(type);
    dest.type = type;
    switch(dest.type){
        case VAR_FLOAT: dest.floatvals[0] = floatvals[0]; break;
        case VAR_INT: dest.intval = intval; break;
        case VAR_BOOL: dest.boolval = boolval; break;
        case VAR_STRING: dest.stringval = stringval; break;
        case VAR_VEC: glm_vec4_copy(floatvals, dest.floatvals);break;
    }
}


void VNVariable::string_infer_cast() {
    if(locked)
        return;

    if( empty() || type != VAR_STRING )
        return;

    // Match int before float when possible
    if( std::regex_match( stringval, regex_int ) ) {
        update_int();
        type = VAR_INT;
    }
    else if( std::regex_match( stringval, regex_float ) ) {
        update_float();
        type = VAR_FLOAT;
    }
    else if( std::regex_match( stringval, regex_vec ) || std::regex_match( stringval, regex_vec_color ) ) {
        update_vec();
        type = VAR_VEC;
    }
    else if( stringval == "true" || stringval == "false" ) {
        update_bool();
        type = VAR_BOOL;
    }
}



void VNVariable::clear() {
    if(locked)
        return;
    valid_flags = 0;
    type = VAR_FLOAT;
    stringval.clear();
    glm_vec4_zero( floatvals );
    intval = 0;
    boolval = false;
}

bool VNVariable::empty()const {
    return valid_flags == 0;
}

/*
 * Set changes value but preserves type
 * Save the old type.
 * Invalidate all types.
 * Set the value of the new type and set to new type.
 * Cast to the old type.
 */

void VNVariable::set( float f ) {
    if(locked)
        return;
    uint8_t t = type;
    valid_flags = 0;
    write_valid(VAR_FLOAT);
    floatvals[0] = f;
    type = VAR_FLOAT;
    cast(t);
}

void VNVariable::set( int i ) {
    if(locked)
        return;
    uint8_t t = type;
    valid_flags = 0;
    write_valid(VAR_INT);
    intval = i;
    type = VAR_INT;
    cast(t);
}

void VNVariable::set( bool b ) {
    if(locked)
        return;
    uint8_t t = type;
    valid_flags = 0;
    write_valid(VAR_BOOL);
    boolval = b;
    type = VAR_BOOL;
    cast(t);
}

void VNVariable::set( vec4 v ) {
    if(locked)
        return;
    uint8_t t = type;
    valid_flags = 0;
    write_valid( VAR_VEC);
    glm_vec4_copy(v, floatvals);
    type = VAR_VEC;
    cast(t);
}

void VNVariable::set( const std::string &s ) {
    if(locked)
        return;
    uint8_t t = type;
    valid_flags = 0;
    write_valid(VAR_STRING);
    stringval = s;
    type = VAR_STRING;
    cast(t);
}

// Returns true if the casted form of the second argument matches
bool VNVariable::equals( VNVariable &other ) {
    switch( type ) {
        case VAR_FLOAT:
            return floatvals[0] == other.value_float();

        case VAR_INT:
            return intval == other.value_int();

        case VAR_BOOL:
            return boolval == other.value_bool();

        case VAR_VEC:
            vec4 other_vec;
            other.value_vec( other_vec );
            return glm_vec4_eqv_eps( floatvals, other_vec );

        case VAR_STRING:
            return stringval == other.value_string();
    }
}

// Value Extractors, return casted values

float VNVariable::value_float() {
    update_float();
    return floatvals[0];
}

int VNVariable::value_int() {
    update_int();
    return intval;
}

bool VNVariable::value_bool() {
    update_bool();
    return boolval;
}

void VNVariable::value_vec( vec4 dest ) {
    update_vec();
    glm_vec4_copy( floatvals, dest );
}

void VNVariable::value_vec3( vec3 dest ) {
    update_vec();
    glm_vec3_copy( floatvals, dest );
}

const std::string &VNVariable::value_string() {
    update_string();
    return stringval;
}

void VNVariable::update_float() {
    if( is_valid( VAR_FLOAT ) )
        return;

    write_valid( VAR_FLOAT );
    switch( type ) {

        case VAR_INT:
            floatvals[0] = intval;
            return;

        case VAR_BOOL:
            floatvals[0] = boolval;
            return;

        case VAR_STRING:
            try {
                floatvals[0] = std::stof( stringval );
            }
            catch( const std::invalid_argument &iva ) {
                floatvals[0] = 0;
            }
            catch( const std::out_of_range &oor ) {
                floatvals[0] = 0;
            }

            return;
    }
}

void VNVariable::update_int() {
    if( is_valid( VAR_INT ) )
        return;

    write_valid( VAR_INT );

    switch( type ) {
        case VAR_BOOL:
            intval = boolval;
            return;

        case VAR_FLOAT:
        case VAR_VEC:
            intval = ( int )floatvals[0];
            return;

        case VAR_STRING:
            try {
                intval = std::stoi( stringval );
            }
            catch( const std::invalid_argument &iva ) {
                intval = 0;
            }
            catch( const std::out_of_range &oor ) {
                intval = 0;
            }

            return;
    }
}

void VNVariable::update_bool() {
    if( is_valid( VAR_BOOL ) )
        return;

    write_valid( VAR_BOOL );

    switch( type ) {
        case VAR_INT:
            boolval = intval > 0;
            return;

        case VAR_FLOAT:
        case VAR_VEC:
            boolval = floatvals[0] > 0;
            return;

        case VAR_STRING:
            boolval =  stringval == "true";
            return;
    }
}

void VNVariable::update_string() {
    if( is_valid( VAR_STRING ) )
        return;

    write_valid( VAR_STRING );

    switch( type ) {

        case VAR_FLOAT:
            stringval = std::to_string( floatvals[0] );
            return;

        case VAR_INT:
            stringval = std::to_string( intval );
            return;

        case VAR_BOOL:
            stringval = boolval ? "true" : "false";
            return;

        case VAR_VEC: {
            stringval.clear();
            stringval.append( "{ " );

            for( uint8_t i = 0; i < 4; ++i ) {
                stringval.append( std::to_string( floatvals[i] ) );
                stringval.push_back( ' ' );
            }

            stringval.push_back( '}' );
            return;
        }
    }
}

void VNVariable::update_vec() {
    if( is_valid( VAR_VEC ) )
        return;

    write_valid( VAR_VEC );

    switch( type ) {
        case VAR_FLOAT:
            floatvals[1] = 0;
            floatvals[2] = 0;
            floatvals[3] = 0;
            return;

        case VAR_INT:
            floatvals[0] = intval;
            floatvals[1] = 0;
            floatvals[2] = 0;
            floatvals[3] = 0;
            return;

        case VAR_BOOL:
            floatvals[0] = boolval;
            floatvals[1] = 0;
            floatvals[2] = 0;
            floatvals[3] = 0;
            return;

        // Attempt to convert various string types into a vector
        case VAR_STRING: {

            // Bool conversion on true/false
            if( stringval == "true" || stringval == "false" ) {
                update_bool();
                write_invalid( VAR_VEC );
                update_vec();
            }

            // Parse a color starting with #
            else if(std::regex_match(stringval,regex_vec_color)){
                // Colors are stored as 3 8-bit channels RGB

                // skip the # char
                const char* substr = &stringval.c_str()[1];
                uint32_t c_buffer = (uint32_t)std::strtoul(substr, NULL, 16);

                // Convert the buffer into float values
                floatvals[0] = ((0x00FF0000&c_buffer)>>16)/255.0;  // R
                floatvals[1] = ((0x0000FF00&c_buffer)>>8)/255.0;   // G
                floatvals[2] = (0x000000FF&c_buffer)/255.0;        // B
                floatvals[3] = 1;                                  // A
            }

            // Parse a space-separated list of int/floats into  a vector
            else if( std::regex_match( stringval, regex_vec ) ) {
                // Create up to 5 possible offsets referencing where there are spaces before a number
                uint16_t spaces[5] = {0, 0, 0, 0};
                uint8_t space_count = 0;

                for( uint16_t i = 0; i < stringval.size(); ++i ) {
                    if( stringval[i] == ' ' && space_count < 5 ) {
                        spaces[space_count] = i;
                        ++space_count;
                    }
                }

                // parse the number at the beginning of each space as a float, the number count is the space count -1
                for( uint8_t i = 0; i < space_count - 1; ++i ) {
                    try {
                        floatvals[i] = std::stof( stringval.substr( spaces[i], spaces[i + 1] ) );
                    }
                    catch( const std::invalid_argument &iva ) {
                        floatvals[i] = 0;
                    }
                    catch( const std::out_of_range &oor ) {
                        floatvals[i] = 0;
                    }
                }

                return;
            }

            // Parse int, try to match int before float
            else if( std::regex_match( stringval, regex_int ) ) {
                update_int();
                write_invalid( VAR_VEC );
                update_vec();
            }

            // Parse Float
            else if( std::regex_match( stringval, regex_float ) ) {
                update_float();
                write_invalid( VAR_VEC );
                update_vec();
            }

            // Unknown, fill with 0
            else {
                glm_vec4_zero( floatvals );
            }

            return;
        }
    }
}

void VNVariable::update_type(uint8_t t){
    switch(t){
        case VAR_FLOAT: update_float(); break;
        case VAR_INT:update_int(); break;
        case VAR_BOOL:update_bool(); break;
        case VAR_STRING:update_string(); break;
        case VAR_VEC:update_vec(); break;
    }
}


