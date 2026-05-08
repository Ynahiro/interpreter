#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "rpn.h"
#include <map>
#include <vector>
#include <string>

class Interpreter {
public:
    void execute(const std::vector<RpnInstruction>& code);

private:
    std::map<std::string, int>              vars;
    std::map<std::string, std::vector<int>> arrays;
    std::vector<int>                        stack;

    void push(int val);
    int  pop();
};

#endif
