#include "OperationDefs.h"
# include "VNDebug.h"
#include "VNInterpreter.h"
#include "VNCompiledFile.h"
#include <sstream>

namespace VNOP{

    std::unordered_map< std::string , OpFuncPtr> operation_map;
    std::unordered_map< OpFuncPtr , OpFormat> format_map;

    void load_ops(){
        load_ops_arithmetic();
        load_ops_armature();
        load_ops_audio();
        load_ops_character();
        load_ops_control();
        load_ops_gui();
        load_ops_image();
        load_ops_model();
        load_ops_object();
        load_ops_shader();
        load_ops_string();
        load_ops_view();

        init_op_formats();
    }

    void init_op_formats(){
        for(auto &opf : format_map){
            opf.second.init();
        }
    }
};

/*
 * Argument Syntax (spaces are separators):
 * keyword (any other format, keywords are not allowed )
 * -var
 * &ref
 * ( enum1 enum2 enum3 enum4 enumetc. ) ( must be space separated list, including parentheses)
 * | -var &ref optional enums, do not count towards the minimum args, but do for the maximum
 * var... allows up to the maximum arg size, must be at end
 * : expects multiple lines
 */

std::regex regex_arg_var = std::regex(R"()");

// Initialize the operation format using the format string
void OpFormat::init() {
    // Tokenize the string
    std::istringstream tokenstream = std::istringstream{format_string};
    std::string token;

    // Marked true while reading enums
    bool enum_listing = false;
    bool optional_set = false;
    bool set_variadic = false;

    // Clear the format
    clear();

    // Check each token for the matching argument type
    while (tokenstream >> token) {
        if(set_variadic || reads_multiple_lines){
            VNDebug::program_error("Variadic and multi-line args must be at end", format_string);
            clear();
            return;
        }

        // Mark as optional
        if(token == "|"){
            optional_set = true;
            min_args = format.size();
        }
        // Mark as optional
        else if(token == ":"){
            reads_multiple_lines = true;
        }
        // Start enum
        else if(token == "("){
            enum_listing = true;
            // Create a new enum arg
            format.push_back({OpFormatArg::ENUM, {}});
        }
        // Stop enum
        else if(token == ")"){
            enum_listing = false;
        }
        // Place as an enum
        else if(enum_listing){
            format.back().enums.push_back(token);
        }
        // Variadic args
        else if(token.ends_with("...")){
            set_variadic = true;
        }
        // Variable
        else if(token.starts_with('-')){
            format.push_back({OpFormatArg::VAR, {}});
        }
        // Reference
        else if(token.starts_with('&')){
            format.push_back({OpFormatArg::REF, {}});
        }
        // Keyword
        else{
            // If args were started, error
            if(!format.empty()){
                VNDebug::program_error("Invalid arg format, can not have another keyword after starting args", format_string);
                clear();
                return;
            }
        }
    }

        if(enum_listing){
            VNDebug::program_error("Invalid arg format, enum never closed", format_string);
            clear();
            return;
        }

        // If variadic, the maximum number of args is set to the defined max (args are indexed by uint8_t)
        if(set_variadic){
            min_args = format.size();
            max_args = MAX_ARGS;
        }
        else{
            max_args = format.size();
            // Sets min args if it was not set by an optional '/'
            if(!optional_set)
                min_args = max_args;
        }
}

void OpFormat::clear() {
    // Do not clear the format string
    format.clear();
    min_args = 0;
    max_args = 0;
    reads_multiple_lines = false;
}


// Validates an operation's arguments
bool OpFormat::validate(VNOperation &op, VNCompiledFile &vncf){
    if(op.args.size() < min_args || op.args.size() > max_args){
        VNDebug::compile_error("Incorect argument count","expected " + std::to_string(min_args) + " to "+ std::to_string(max_args) + " got " + std::to_string(op.args.size()), vncf);
        return false;
    }

    // When checking formats, only the first listed formats are checked, remaining values are ignored and treated as args
    for(uint8_t i = 0; i < op.args.size() && i < format.size(); ++i){
        // Verify the arguments of each format type
        switch(format[i].type){
            // Nothing needs checked
            case OpFormatArg::VAR:
                break;

            // Check to see if the string value matches one of the enums, if not error and return
            case OpFormatArg::ENUM:{
                bool found = false;
                for(uint8_t j = 0; j < format[i].enums.size(); ++j){
                    if(format[i].enums[j] == op.args[i].value_string()){
                        found = true;
                        break;
                    }
                }
                if(!found){
                    VNDebug::compile_error("No matching enum found", op.args[i].value_string(), vncf);
                    return false;
                }
                break;
            }

            // Check if argument has a reference variable, if not error and return
            case OpFormatArg::REF:
                if(op.args[i].var_id == 0){
                    VNDebug::compile_error("Argument is not a reference", op.args[i].value_string(), vncf);
                    return false;
                }
            break;
        }
    }

    return true;
}
