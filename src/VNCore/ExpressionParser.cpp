#include "ExpressionParser.h"

std::regex operand_inline = std::regex(R"((^\-?(?:[0-9]+)?(?:\.?[0-9]+))(.*$))");
std::regex operand_ref = std::regex(R"((^[a-zA-Z]+[a-zA-Z0-9_]*)(.*$))");
std::regex whitespace = std::regex(R"((\s+)(.*$))");

enum expr_ops:uint8_t {
    op_push,
    op_pow,
    op_mul,
    op_div,
    op_mod,
    op_add,
    op_sub,
    op_eq,
    op_neq,
    op_and,
    op_or,
    op_lt,
    op_gt,
    op_lte,
    op_gte,

    // 1 Arg
    op_abs,
    op_not,
    op_ceil,
    op_flr,
    op_cos,
    op_sin,
    op_tan,
    op_acos,
    op_asin,
    op_atan,

    // 2 Args
    op_min,
    op_max,
    op_rand
};

struct expr_operator_info {
    expr_ops id;
    int8_t precedence;
    bool right_associative = false;
};

const std::unordered_map<std::string, expr_operator_info> operator_orders={
    {"(", {op_push,-1}},   // Stack Push

    {"!", {op_not,8, true}},            // Bool Negation

    {"^", {op_pow,7}},            // Power

    {"*", {op_mul,6}},            // Multiply
    {"/", {op_div,6}},            // Divide
    {"%", {op_mod,6}},            // Mod

    {"+", {op_add,5}},            // Add
    {"-", {op_sub,5}},            // Subtract

    {"==",{op_eq,4}},             // Equals
    {"!=",{op_neq,4}},            // Not-Equal

    {"<", {op_lt,3}},             // Less Than
    {">", {op_gt,3}},             // Greater Than
    {"<=", {op_lte,3}},           // Less Than Equal To
    {">=", {op_gte,3}},            // Greater Than Equal To

    {"&", {op_and,2}},            // Boolean And

    {"|", {op_or,1}},             // Boolean Or


    {"abs", {op_abs, 0 ,true}},
    {"min", {op_min, 0 ,true}},
    {"max", {op_max, 0 ,true}},
    {"rand", {op_rand, 0 ,true}},
    {"ceil",{op_ceil, 0 ,true}},
    {"flr",{op_flr, 0 ,true}},

    {"sin",{op_sin}},
    {"cos",{op_cos}},
    {"tan",{op_tan}},
    {"acos",{op_acos}},
    {"asin",{op_asin}},
    {"atan",{op_atan}},
};

bool ExpressionParser::parse_expression(const std::string &exprstr, VNOperation &vnop, VNCompiledFile &vncf){
    std::smatch match;
    std::stack<expr_operator_info> opstk;
    std::string ex = exprstr;


    // Use Djkstra Shunting-Yard algorithm
    while(!ex.empty()){

        // Ignore whitespace
        if(std::regex_match(ex,match,whitespace)){
            ex.erase(ex.begin(), ex.begin()+match.length(1));
            continue;
        }

        // Open stack
        if(ex.starts_with('(')){
            opstk.push({op_push,-1});
            ex.erase(0,1);
            continue;
        }

        // Close stack
        if(ex.starts_with(')')){
            while(!opstk.empty() && opstk.top().id != op_push){
                vnop.args.push_back((int)(opstk.top().id));
                opstk.pop();
            }
            if(opstk.empty()){
                VNDebug::compile_error("Unmatched ) ", ex, vncf);
                return false;
            }
            // Pop the remaining (
            opstk.pop();
            ex.erase(0,1);
            continue;
        }

        // Arg sep
        if(ex.starts_with(',')){
            while(!opstk.empty() && opstk.top().id != op_push){
                vnop.args.push_back((int)(opstk.top().id));
                opstk.pop();
            }
            if(opstk.empty()){
                VNDebug::compile_error("Arg seperator , not enclosed", ex, vncf);
                return false;
            }
            ex.erase(0,1);
            continue;
        }


        // Read an inline
        else if(std::regex_match(ex, match, operand_inline)){
            vnop.args.push_back(match.str(1));
            vnop.args.back().cast(VAR_FLOAT);
            ex.erase(0, match.length(1));
            continue;
        }

        // Operator (size 1-4)
        bool op_found = false;
        for(uint8_t i = 4; i > 0; --i){
            try{
                expr_operator_info op = operator_orders.at(ex.substr(0,i));
                if(op.right_associative){
                    while( !opstk.empty() && opstk.top().precedence > op.precedence ) {
                        vnop.args.push_back( (int)opstk.top().id );
                        opstk.pop();
                    }
                }
                else{
                    while( !opstk.empty() && opstk.top().precedence >= op.precedence ) {
                        vnop.args.push_back( (int)opstk.top().id );
                        opstk.pop();
                    }
                }

                opstk.push( op );
                ex.erase( 0, i );
                op_found = true;
                break;
            }
            catch(std::out_of_range &oor){
            }
        }
        if(op_found)
            continue;

        // Read a reference, this is done after reading operators to prevent shadowing
        if(std::regex_match(ex,match, operand_ref)){
            vnop.args.push_back( VNVariable(match.str(1)));

            uint8_t id = VNI::variables.get_id( match.str(1) );
            if( id == 0 ) {
                VNDebug::compile_error( "Variable was not declared.", match.str(1), vncf);
                return false;
            }
            else
                vnop.args.back().var_id = id;

            ex.erase(0, match.length(1));
            continue;
        }

        VNDebug::compile_error("Unable to match remaining expression", ex, vncf);
        return false;

    }

    // Flush the remaining operators to the arguments
    while(!opstk.empty()){
        if(opstk.top().id == op_push){
            VNDebug::compile_error("Unmatched ( ", ex, vncf);
            return false;
        }

        vnop.args.push_back((int)opstk.top().id);
        opstk.pop();
    }

    return validate_expr( vnop, vncf);
}

bool ExpressionParser::validate_expr(VNOperation &op, VNCompiledFile &vncf){
    // TODO
    printf("EXPR: ");
    for(VNVariable &v : op.args){
        printf("%d:%s ", v.get_type(), v.value_string().c_str());
    }
    printf("\n");
    fflush(stdout);
    return true;
}

float ExpressionParser::run_expression(uint8_t start, std::vector<VNVariable> &args, VNInterpreter &vni){
    std::stack<float> operands;
    float a,b,c;

    for(uint8_t i = start; i < args.size(); ++i){

        // Variable operand
        if(args[i].var_id != 0){
            operands.push(args[i].value_float());
        }
        else{
            // Operator
            if(args[i].get_type() == VAR_INT){

                // 1 OP
                if(operands.size() < 1){
                    VNDebug::runtime_error("Bad Expression","Not enough operands:1",vni);
                    return 0;
                }
                b = operands.top();
                operands.pop();

                bool set = true;
                switch(args[i].value_int()){
                    case op_abs:   c = abs(b);    break;
                    case op_not:   c = !b;        break;
                    case op_ceil:  c = ceil(b);   break;
                    case op_flr:  c = floor(b);   break;
                    case op_cos:   c = cos(b);    break;
                    case op_sin:   c = sin(b);    break;
                    case op_tan:   c = tan(b);    break;
                    case op_acos:  c = acos(b);   break;
                    case op_asin:  c = asin(b);   break;
                    case op_atan:  c = atan(b);   break;
                    default : set = false; break;
                }

                if(set){
                    operands.push(c);
                    continue;
                }

                // 2 OP
                if(operands.size() < 1){
                    VNDebug::runtime_error("Bad Expression","Not enough operands:2",vni);
                    return 0;
                }
                a = operands.top();
                operands.pop();

                set = true;
                switch(args[i].value_int()){
                    case op_pow:   c = pow(a,b); break;
                    case op_mul:   c = a * b; break;
                    case op_div:   if(b!=0)c = a / b; break;
                    case op_mod:   c = fmod(a,b); break;
                    case op_add:   c = a + b; break;
                    case op_sub:   c = a - b; break;
                    case op_eq:    c = a == b; break;
                    case op_neq:   c = a != b; break;
                    case op_and:   c = a && b; break;
                    case op_or:    c = a || b; break;
                    case op_lt:    c = a < b; break;
                    case op_gt:    c = a > b; break;
                    case op_lte:   c = a <= b; break;
                    case op_gte:   c = a >= b; break;
                    case op_min:   c = fmin(a,b); break;
                    case op_max:   c = fmax(a,b); break;
                    case op_rand:  c = (rand()*(b-a)/(float)RAND_MAX)+a; break;
                    default : set = false; break;
                }

                if(set){
                    operands.push(c);
                    continue;
                }
                else{
                    VNDebug::program_error("Unknown expression operator was enocuntered, this should not have happened!", args[i].value_string().c_str());
                    return 0;
                }

            }

            // Constant Operand
            else{
                operands.push(args[i].value_float());
            }
        }
    }
    if(operands.size() == 1){
        return operands.top();
    }
    else{
        VNDebug::runtime_error("Expression validation did not create a valid expression, operands left over", std::to_string(operands.size()), vni);
        return 0;
    }

}
