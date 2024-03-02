#include "VNOperation.h"
#include "VNOperationDefs/OperationDefs.h"
#include <unordered_map>
#include "VNInterpreter.h"

void VNOperation::load_args() {
    if( !function_ptr )
        return;

    for( uint8_t i = 0; i < args.size(); ++i ) {
        if( args[i].var_id != 0 )
            VNI::variables.at( args[i].var_id ).copy( args[i] );
    }

}


bool VNOperation::construct( const std::vector<std::string> &tokens, uint8_t offset, VNCompiledFile &vncf ) {

    // Clear the operation
    function_ptr = nullptr;
    args.clear();
    line_number = vncf.get_line_number();

    // Extract the first token
    if( tokens.empty() )
        return false;

    std::string op_name;

    // merge 2 tokens
    if(
        tokens.size() >= 2 && (
            tokens[offset] == "menu" ||
            tokens[offset] == "element" ||
            tokens[offset] == "audio" ||
            tokens[offset] == "music" ||
            tokens[offset] == "action" ||
            tokens[offset] == "view" ||
            tokens[offset] == "shader" ||
            tokens[offset] == "model" ||
            tokens[offset] == "object" ||
            tokens[offset] == "armature" ||
            tokens[offset] == "image" ||
            tokens[offset] == "say" ||
            tokens[offset] == "character" ||
            tokens[offset] == "quat" ||
            tokens[offset] == "str"
        )
    ) {
        op_name = tokens[offset];
        op_name.append( " " + tokens[offset + 1] );
        offset += 2;
    }
    // 1 token
    else {
        op_name = tokens[offset];
        offset += 1;
    }

    // This allows for presetting unregistered operations made by the vncf
    if(function_ptr == nullptr){
        try {
            function_ptr = VNOP::operation_map.at( op_name );
        }
        catch( std::out_of_range &oor ) {
            VNDebug::compile_error( "Unknown operation.", op_name, vncf );
            return false;
        }
    }

    init_args(tokens, offset, vncf);

    return validate(vncf);
}

bool VNOperation::validate(VNCompiledFile &vncf) {
    if(!VNOP::format_map[function_ptr].validate(*this,vncf)){
        VNDebug::format_assistance(VNOP::format_map[function_ptr].format_string);
        return false;
    }
    return true;
}

bool VNOperation::init_args(const std::vector<std::string> &tokens, uint8_t offset, VNCompiledFile &vncf){
    // Copy the remaining arguments as variables, those beginning with & are references
    for( uint8_t i = offset; i < tokens.size(); ++i ) {

        VNVariable v;
        v.cast( VAR_STRING );

        // Get the vtable value and save the argument location
        if( tokens[i].starts_with( '&' ) ) {
            v.set( tokens[i].substr( 1, tokens[i].size() ) );
            uint8_t id = VNI::variables.get_id( v.value_string() );

            if( id == 0 ) {
                VNDebug::compile_error( "Variable was not declared.", v.value_string(), vncf );
                return false;
            }
            else
                v.var_id = id;
        }

        // Parse the variable as an inline value instead, lock them so they are not accidentally changed
        else {
            v.set( tokens[i] );
            v.string_infer_cast();
            v.lock();
        }

        args.push_back( v );
    }
}
