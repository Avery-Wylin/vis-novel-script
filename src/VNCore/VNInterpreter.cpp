#include "VNInterpreter.h"
#include <iterator>
#include <regex>
#include <chrono>
#include <thread>
#include <pthread.h>
#include "VNOperationDefs/OperationDefs.h"

namespace VNI{
    Window window;
    VNVariableContainer variables;
    VNInterpreter main_interpreter;
    std::unordered_map<std::string,VNCompiledFile> compiled_files;
    std::unordered_map<std::string,VNOperation> aliases;
    std::string elem_text = "text", elem_name = "name";
    bool stopped = false;

    bool is_waiting = false, wait_skippable = false;
    float wait_time = 0;

    // Should be called by window thread
    void wait(){
        is_waiting = true;
    }

    void wait(float time, bool can_skip){
        // Time can not be manipulated if not-skippable
        if(!wait_skippable && wait_time > 0)
            return;
        wait_skippable = can_skip;
        wait_time = time;
    }

    // Should be called by window thread
    void resume(){
        if(wait_skippable)
            wait_time = 0;
        is_waiting = false;
    }

    // Recompiles the current file for the main VNI
    void recompile(){
        main_interpreter.recompile_vncf();
    }

    void update(float time){

        // Skip main interpreter if waiting or stopped
        if( is_waiting || stopped){
            return;
        }

        // If there is a wait time, tick it down and skip
        if(wait_time > 0){
            wait_time -= time;
            return;
        }

        while(VNI::main_interpreter.execute_next() && !stopped ){
            if(is_waiting || wait_time > 0)
                return;
        }
        // If this section is reached, the VNI was either stopped or reached eof
        VNDebug::message("VNI has ended or was interupted in file",main_interpreter.get_current_file());
        stopped = true;

    }

    void stop(){
        stopped = true;
    }

    void open_file(std::string name){
        // Load the file, erase the entry on fail, file deletion is not supported after loading (could cause trash ptr)
        if(compiled_files.contains(name))
            return;
        if(!compiled_files[name].load(name)){
            compiled_files.erase(name);
        }
    }
};

// Switches the currently executing file to another, does nothing on fail
// switching the file does not call skip() on fail
// some instructions need skipped such as jump, branch, and return
bool VNInterpreter::switch_file(std::string filename, bool link_return){
    if(current_file == filename)
        return true;

    if(is_routine){
        puts("Routines may not switch file.");
        fflush( stdout );
        return false;
    }

    try {
        vncf = &VNI::compiled_files.at( filename );
        if(link_return)
            last_jump_file = current_file;
        current_file = filename;
    }
    // The operation failed because the file was not found, try to open the file
    catch( std::out_of_range &oor ) {
        VNI::open_file(filename);
        printf( "Opening file %s\n", filename.c_str() );
        fflush( stdout );
    }

    // Try again, if it fails no further attempt is made
    try {
        vncf = &VNI::compiled_files.at( filename );
        if(link_return)
            last_jump_file = current_file;
        current_file = filename;
    }
    catch( std::out_of_range &oor ) {
        printf( "Switching to file %s failed\n", filename.c_str() );
        fflush( stdout );
        return false;
    }

    return true;
}

// Jump to a label in the current file, assured safe, ignores nothing on fail
void VNInterpreter::jump( std::string const &label, bool link_return ) {
    if(!vncf)
        return;
    int line = -1;
    try{
        line = vncf->get_labels().at(label);
    }
    catch(std::out_of_range &oor){
        printf("Unable to find label %s for jump\n", label.c_str());
        fflush(stdout);
        // Skip the jump if it failed
        skip();
    }
    jump(line, link_return);
}

// Jump to a line in the current file, assured safe, ignores on fail
void VNInterpreter::jump(int32_t line, bool link_return){
    if(!vncf || line < 0 || line >= vncf->get_operations().size()){
        printf("Jump to line %d failed\n", line);
        fflush(stdout);
        // If a jump fails, it must skip the operation, otherwise it will keep calling.
        skip();
        return;
    }
    if(link_return){
        last_jump_line = execution_line;
    }
    execution_line = line;
}

// Jumps to the last linked jump, labels are assured linked
bool VNInterpreter::jump_return(){
    if(switch_file(last_jump_file, false)){
        // The jump will skip if it fails
        jump(last_jump_line, false);
        return true;
    }
    else{
        // Call a skip on failure to switch file
        skip();
        return false;
    }
}

// Executes a line and returns whether or not the interpreter has more lines
bool VNInterpreter::execute_next(){
    if(!vncf)
        return false;

    // Ensure operation in-bounds
    if(execution_line >= vncf->get_operations().size())
        return false;

    VNOperation& op = vncf->get_operations()[execution_line];

    // // If a jump function, do not increment, it will jump to the line it needs
    // // jump return is an exception, otherwise it would jump to the jump, thus looping endlessly
    if(
        op.function_ptr == VNOP::jump   ||
        op.function_ptr == VNOP::branch ||
        op.function_ptr == VNOP::jumpto ||
        op.function_ptr == VNOP::ifbranch
    ){
        op.run(*this);
        // Jumps handle their own skips
        return execution_line < vncf->get_operations().size();
    }

    // Skip nested routines
    if(is_routine && op.function_ptr == VNOP::routine){
        ++execution_line;
        return true;
    }

    op.run(*this);
    ++execution_line;
    return execution_line < vncf->get_operations().size();
}

void VNInterpreter::start_routine(std::string &filename, std::string &label){

    // Temporarily remove routine status to switch files
    is_routine = false;

    switch_file( filename, true );
    if(!vncf){
        puts("Error: Routine could not find compiled file.");
        fflush(stdout);
        return;
    }

    // Allows for jump returns within the file
    last_jump_file = filename;

    // Ensure labels are in the file, otherwise return
    int start = -1;
    try{
        start = vncf->get_labels().at(label);
    }
    catch(std::out_of_range &oor){
        printf("Routine unable to find labels %s in file %s\n", label.c_str(), filename.c_str());
        fflush(stdout);
        return;
    }

    jump( start, true);

    // Switch to routine mode
    is_routine = true;

    while(execute_next());

    // The routine will still exit if it hits an end
}

void VNInterpreter::recompile_vncf() {
    if(!vncf)
        return;
    VNDebug::message("Recompiling file", current_file);
    vncf->load(current_file);
}

void VNInterpreter::exit() {
    if(!vncf)
        return;
    // Force the line value to the end, thus ending execute_next()
    // The last line is not called because this is called before a the line is incremented
    execution_line = vncf->get_operations().size()-1;
}

const VNOperation& VNInterpreter::get_current_op(){
    return vncf->get_operations()[execution_line];
}
