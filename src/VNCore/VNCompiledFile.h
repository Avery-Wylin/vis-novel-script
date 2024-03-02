#ifndef VNCOMPILEDFILE_H
#define VNCOMPILEDFILE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include "VNOperation.h"


/**
 * Converts a raw file into a compiled file. Can read a file and convert it into tokens.
 */
class VNCompiledFile {

    // uint32_t line_number;
    // int32_t execution_line = 0;
    std::string filename;
    std::string multi_line_string;
    std::vector<std::string> tokens;
    std::vector<VNOperation> operations;
    std::unordered_map<std::string, int32_t> labels;
    std::string line_in;

    uint32_t line_number = 0;
    // bool finished = false;

    void compile();
    void merge_between(char start, char stop, bool trim = false);
    bool variable_def();
    bool control_flow();
    bool expression();

    std::stack<int32_t> cf_stack;
    std::stack<std::vector<int32_t>> end_jump_stack;

public:
    bool reading_multi_line = false;
    bool load(std::string file);
    inline std::vector<VNOperation>& get_operations(){return operations;};
    inline const std::unordered_map<std::string, int32_t>& get_labels(){return labels;};
    inline uint32_t get_line_number(){return line_number;};
    inline const std::string& get_file(){return filename;};
};

#endif // VNCOMPILEDFILE_H
