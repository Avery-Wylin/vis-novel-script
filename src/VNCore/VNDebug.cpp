#include "VNDebug.h"
#include "VNInterpreter.h"
#include "VNCompiledFile.h"

namespace VNDebug {

    bool enabled = true;


    void compile_error( const char *msg, const std::string &msg2, VNCompiledFile &vncf ) {
        if( enabled )
            printf( "\033[31mCompilation Error %s %d: %s : %s \033[0m\n", vncf.get_file().c_str(), vncf.get_line_number(), msg, msg2.c_str() );
    }

    void runtime_error( const char *msg, const std::string &msg2,  VNInterpreter &vni ) {
        if( enabled )
            printf( "\033[31mRuntime Error %s %d: %s : %s \033[0m\n", vni.get_current_file().c_str(), vni.get_current_op().line_number, msg, msg2.c_str() );
    }

    void format_assistance( const char *assistance ) {
        if(assistance)
            printf("\033[36mUse Format: %s \033[0m\n",assistance);
    }

    void program_error(const char* error, const char* msg){
        printf("\033[31mProgram Error %s : %s\033[0m\n",error, msg);
    }

    void warning(const char* warning, const char* msg){
        printf("\033[33mWarning %s : %s\033[0m\n",warning, msg);
    }

    void message(const char* msg, const std::string &msg2){
        printf("\033[32mVNI: %s %s\033[0m\n",msg, msg2.c_str());
    }

}
