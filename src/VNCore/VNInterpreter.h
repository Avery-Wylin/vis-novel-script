#ifndef VNINTERPRETER_H
#define VNINTERPRETER_H

#include "VNDebug.h"
#include "definitions.h"
#include <VNCompiledFile.h>
#include <Window.h>
#include "VNVariable.h"
#include <pthread.h>


class VNCompiledFile;

class VNInterpreter{
    VNCompiledFile *vncf = nullptr;
    int32_t last_jump_line = 0;
    std::string last_jump_file;
    std::string current_file;
    int32_t execution_line = 0;

public:
    std::string match_filter;
    bool filtered = false;
    bool matching = false;
    bool is_routine = false;

    bool switch_file(std::string filename, bool link_return);
    void jump( std::string const &label, bool link_return = true);
    void jump( int32_t line, bool link_return);
    bool jump_return();
    bool execute_next();
    void start_routine(std::string &filename, std::string &label);
    void recompile_vncf();
    void exit();
    inline void skip(){++execution_line;}
    const VNOperation& get_current_op();
    inline const std::string &get_current_file(){
        return current_file;
    }
};

namespace VNI{
    extern Window window;
    extern VNVariableContainer variables;
    extern VNInterpreter main_interpreter;
    extern std::unordered_map<std::string,VNCompiledFile> compiled_files;
    extern std::string elem_text, elem_name;

    // wait and wake should be called from external threads
    void wait();
    void wait(float time, bool can_skip = true);
    void update(float time);
    void resume();
    void recompile();
    void stop();
    void open_file(std::string name);
};

#endif // VNINTERPRETER_H
