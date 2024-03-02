#ifndef VNDEBUG_H
#define VNDEBUG_H

#include <string>
class VNInterpreter;
class VNCompiledFile;

namespace VNDebug{

    extern bool enabled;

    void compile_error( const char* msg, const std::string& msg2, VNCompiledFile &vncf);
    void runtime_error( const char* msg, const std::string& msg2, VNInterpreter &vni);
    void format_assistance(const char* assistance);
    void program_error(const char* error, const char* msg);
    void warning(const char* warning, const char* msg);
    void message(const char* msg, const std::string &msg2);
}

#endif //VNDEBUG_H
