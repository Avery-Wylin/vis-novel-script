#include "definitions.h"
#include <fstream>
#include <sstream>
#include <iterator>
#include "VNInterpreter.h"
#include "VNCompiledFile.h"
#include "VNDebug.h"
#include "ExpressionParser.h"

bool VNCompiledFile::load( std::string file ){

    operations.clear();
    labels.clear();
    cf_stack = std::stack<int32_t>();
    end_jump_stack = std::stack<std::vector<int32_t>>();

    // Open the script file to run
    filename = file;
    std::string filepath = ( std::string )DIR_SCRIPTS + filename + ".vnas";
    std::ifstream filereader;
    filereader.open( filepath, std::ios::in );

    // Stop if the script could not be opened
    if( !filereader.is_open() ) {
        printf( "Unable to open script %s\n", filepath.c_str() );
        fflush( stdout );
        return false;
    }

    std::stringstream tokenstream;

    line_number = 0;

    // Read each line and process it
    while( !filereader.eof() ){

        // Used for debug printing
        line_number++;

        // Get the line, then separate it into a token list using istream_iterator
        std::getline(filereader, line_in, '\n');

        // If the line is empty, stop reading multiple lines, pass the string to the operation dest, and continue
        if(line_in.empty()){
            if( reading_multi_line ){
                reading_multi_line = false;

                // If you forgot to put a string on the next line (it happens)
                if(multi_line_string.empty()){
                    operations.back().args.push_back("");
                    continue;
                }

                // Remove the last new line, for it is not needed
                multi_line_string.pop_back();

                // Multi-line operations are added as a final argument, no validation is performed
                operations.back().args.push_back(multi_line_string);
                multi_line_string.clear();
            }
            continue;
        }

        // If reading multiple lines, append the line to the multi-line string and continue
        if( reading_multi_line ){
            multi_line_string.append(line_in+"\n");
            continue;
        }

        // Tokenize the line_in string
        tokenstream.str( line_in );
        if(tokenstream.fail())
            tokenstream.clear();
        tokens = std::vector<std::string>( std::istream_iterator<std::string>( tokenstream ), {} );

        // Pass on for compilation
        compile();
    }

    filereader.close();
    return true;
}

// Merges all tokens starting with a one character and ending in another (inclusive), optionally trims off ends
void VNCompiledFile::merge_between( char start, char stop, bool trim){


    for( uint32_t i = 0; i < tokens.size(); ++i ) {

        // Empty case
        if(trim && tokens[i].size() == 2 && tokens[i].starts_with(start) && tokens[i].ends_with(stop)){
            tokens[i].clear();
        }

        // Token matches opening
        if( tokens[i].size() == 1 && tokens[i][0] == start ) {


            while(true){
                if( i+1 >= tokens.size()){
                    VNDebug::compile_error("Merging tokens failed", tokens[i], *this);
                    return;
                }

                tokens[i].append(" " + tokens[i+1]);

                if( tokens[i+1].size() == 1 && tokens[i+1][0] == stop ){

                    // trim "start space" then the following "space stop space"
                    if( trim )
                        tokens[i] = tokens[i].substr( 2, tokens[i].size() - 4 );

                    tokens.erase( tokens.begin() + i + 1);
                    break;
                }

                tokens.erase( tokens.begin() + i + 1);
            }
        }
    }
}

// Handles the declaration of a variable
bool VNCompiledFile::variable_def(){
    if(tokens[0] != "var")
        return false;

    // Must have 2 tokens, and a valid variable name
    if( tokens.size() < 2 || !std::regex_match(tokens[1],regex_var_name)){
        VNDebug::compile_error("Bad variable name", line_in, *this);
        return false ;
    }

    VNVariable v;

    // If 3+ tokens, use the 3rd token as an initializer and infer cast
    if(tokens.size() >= 3){
        v.cast(VAR_STRING);
        v.set(tokens[2]);
        v.string_infer_cast();
    }

    // Place int the variable table if it does not exist
    if(!VNI::variables.contains(tokens[1]))
        VNI::variables.place(tokens[1], v);

    return true;
}

bool VNCompiledFile::control_flow(){
    VNOperation op;

    if( tokens[0] == "if" ) {

        // Add an ifbranch operation, insert an implied jump line arg and validate
        op = {VNOP::ifbranch, line_number};
        op.init_args(tokens, 1 , *this);
        op.args.insert(op.args.begin(),VNVariable(-1));
        if(op.validate(*this))
            operations.push_back(op);
        else
            return true;

        // Place on to the stack
        cf_stack.push(operations.size()-1);

        // Add a new end-jump list to the stack
        end_jump_stack.push({});

        return true;
    }
    else if( tokens[0] == "elseif") {

        if(cf_stack.empty() || end_jump_stack.empty()){
            VNDebug::compile_error("No matching if/elseif for",line_in,*this);
            return true;
        }

        // Create a new end-jump
        operations.push_back({VNOP::jumpto, line_number, {VNVariable(-1)}});
        end_jump_stack.top().push_back(operations.size()-1);

        // Add an ifbranch
        op = {VNOP::ifbranch, line_number};
        op.init_args(tokens, 1 , *this);
        op.args.insert(op.args.begin(),VNVariable(-1));
        if(op.validate(*this))
            operations.push_back(op);
        else
            return true;

        // Set the previous if/elseif to jump here on fail
        operations[cf_stack.top()].args[0].set((int)operations.size()-1);
        cf_stack.pop();

        // Place on to the cf stack
        cf_stack.push(operations.size()-1);

        return true;
    }
    else if( tokens[0] == "else" ) {

        if(cf_stack.empty() || end_jump_stack.empty()){
            VNDebug::compile_error("No matching if/elseif for",line_in,*this);
            return true;
        }

        // Create a new end-jump
        operations.push_back({VNOP::jumpto, line_number, {VNVariable(-1)}});
        end_jump_stack.top().push_back(operations.size()-1);

        // Add a no-op
        operations.push_back({nullptr, line_number, {VNVariable()}});
        // operations.push_back({VNOP::jumpto, line_number,{VNVariable(-1)}});

        // Set the previous if/elseif to jump here on fail
        operations[cf_stack.top()].args[0].set((int)operations.size()-1);
        cf_stack.pop();

        // Place on to the cf stack
        cf_stack.push(operations.size()-1);

        return true;
    }
    else if( tokens[0] ==  "end" ) {
        if( tokens.size() > 1 ) {
            VNDebug::compile_error( "end statement accepts 0 args ", line_in, *this );
        }

        if(cf_stack.empty() || end_jump_stack.empty()){
            VNDebug::compile_error("No matching if/elseif for",line_in,*this);
            return true;
        }

        // Add a no-op target
        operations.push_back(VNOperation());

        // Link the last value in the cf stack to jump here on fail and pop
        operations[cf_stack.top()].args[0].set((int)operations.size()-1);
        cf_stack.pop();

        // Link all end-jumps to this operations
        for(int32_t i : end_jump_stack.top()){
            operations[i].args[0].set((int)operations.size()-1);
        }
        end_jump_stack.pop();

        return true;
    }

    return false;
}

// Creates an expression using the offset as the destination value, returns true if successful
bool VNCompiledFile::expression(uint32_t offset, VNOperation &op){
    if(tokens.size()-offset < 2)
        return false;

    op = {VNOP::expr, line_number};

    // Place the first arg
    uint8_t vid = VNI::variables.get_id(tokens[offset]);
    if(!vid){
        VNDebug::compile_error("Undeclared variable", tokens[offset],*this);
        return false;
    }
    else{
        op.args.push_back(VNVariable(tokens[offset]));
        op.args.back().var_id = vid;
    }

    // Merge back remaining tokens and pass to parser
    std::string ex;
    for(uint8_t i = offset+1; i < tokens.size(); ++i){
        ex.append(tokens[i]+" ");
    }

    // If the parser was successful, append the operation
    if(ExpressionParser::parse_expression(ex, op, *this)){
        return true;
    }

    return false;

}

// Compiles a set of tokens into a VNOperation which can be easily called
void VNCompiledFile::compile(){

    // Assured to have at least one token

    // ignore comments
    if(tokens[0].starts_with('/'))
        return;

    // Check for labels and save them
    // labels are not linked by id to a jump, this allows jumps to go forward and be set by variable
    if(tokens.size() == 2 && (tokens[0] == "#" || tokens[0] == "label")){
        // Push back a null operation so there is a target to jump to
        labels[tokens[1]] = operations.size();
        operations.push_back(VNOperation());
        return;
    }

    // Merge between brackets and quotes, trim the quotes off
    merge_between('{','}');
    merge_between('"','"', true);

    // Create an alias reference
    if(tokens[0].starts_with('$')){
        const std::string& name = tokens[0].substr(1,tokens[0].size()-1);
        if(VNI::aliases.contains(name)){
            VNOperation op;
            op.line_number = line_number;

            // Make a clone if a multi-line op
            OpFunc *f = VNI::aliases[name].function_ptr;
            if(VNOP::format_map[f].reads_multiple_lines){

                // Copy the operator, mark for multi-line reading which will set the last arg
                op = VNI::aliases[name];
                reading_multi_line = true;
            }
            // Make an alias reference to the operator
            else{
                op.function_ptr = VNOP::alias;
                op.args.push_back(name);
            }

            operations.push_back(op);
            return;
        }
        else{
            VNDebug::compile_error("Alias not defined", line_in.substr(1,line_in.size()-1), *this);
            return;
        }
    }

    // Define an alias
    if(tokens[0] == "alias"){
        if(tokens.size() < 3){
            VNDebug::compile_error("Aliases require a name and an operation", line_in, *this);
            return;
        }

        VNOperation op;
        op.line_number = line_number;

        // Create an expression if it matches
        if( tokens[2] == "expr" ) {
            if( expression( 3, op ) )
                VNI::aliases[tokens[1]] = op;
            return;
        }

        // If the creation of the operation failed, return
        if(op.construct(tokens, 2, *this)){
            VNI::aliases[tokens[1]] = op;
            return;
        }
    }

    // Shorthand for say operator using @(variable name)
    if(tokens[0].starts_with('@')){
        // Convert the @ into an &
        tokens[0][0] = '&';
        VNOperation op;
        op.line_number = line_number;

        // If exactly one token, use say_start
        if(tokens.size() == 1){
            op.function_ptr = VNOP::say_start;
        }
        // If more than one token, use say_print
        else{
            op.function_ptr = VNOP::say_print;
        }

        // Validation is not necessary (no room for syntax error)
        if(!op.init_args(tokens, 0, *this))
            return;

        if(VNOP::format_map[op.function_ptr].reads_multiple_lines)
            reading_multi_line = true;

        operations.push_back(op);
        return;
    }


    // Compile an expression if encountered
    if(tokens[0]=="expr"){
        VNOperation op;
        op.line_number = line_number;
        if(expression(1,op)){
            operations.push_back(op);
        }
        return;
    }

    // Compile a control flow statement
    if( control_flow() )
        return;

    // Check for variable definitions, a variable op still exist and simply sets the value
    variable_def();

    // Pass the rest on to the operation creator
    VNOperation op;
    if(op.construct(tokens, 0, *this)){

        // Set the vncf to read multiple files if the operation format specifies it
        if(VNOP::format_map[op.function_ptr].reads_multiple_lines)
            reading_multi_line = true;

        operations.push_back(op);
    }

}


