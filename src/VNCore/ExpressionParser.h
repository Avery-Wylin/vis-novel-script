#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "VNVariable.h"
#include "VNInterpreter.h"
#include <string>

namespace ExpressionParser
{
    // Parses an expression string into a series of arguments
    bool parse_expression(const std::string &ex, VNOperation &op, VNCompiledFile &vncf);

    // Runs an expression that was parsed into postfix
    float run_expression(uint8_t start, std::vector<VNVariable> &args, VNInterpreter &vni);

    // Validate and optimize an expression by removing operations that are constants
    bool validate_expr(VNOperation &op, VNCompiledFile &vncf);
};

#endif // EXPRESSIONPARSER_H
