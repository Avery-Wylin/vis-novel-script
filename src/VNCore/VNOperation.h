#ifndef VNOPERATION_H
#define VNOPERATION_H

#include "definitions.h"
#include "VNVariable.h"
#include "VNOperationDefs/OperationDefs.h"
#include <string>
#include <vector>
#include <regex>

// Forward Declarations
struct VNOperation;
class VNInterpreter;
class VNCompiledFile;


struct VNOperation {
        OpFuncPtr function_ptr = nullptr;
        uint32_t line_number = 0;
        std::vector<VNVariable> args;

        // References are specified by the '&' character before the variable name
        // Constructs the operation from a token stream
        // The operation may not be valid and is only checked in run-time
        bool construct( const std::vector<std::string> &tokens, uint8_t offset, VNCompiledFile &vncf);

        bool init_args(const std::vector<std::string> &tokens, uint8_t offset, VNCompiledFile &vncf);

        bool validate(VNCompiledFile &vncf);
        bool has_op(){ return function_ptr != nullptr; };

        void load_args();

        // Runs the operation
        inline void run( VNInterpreter &vni ) {
            if(!function_ptr)
                return;
            load_args();
            function_ptr(args,vni);
        };

};

#endif // VNOPERATION_H
