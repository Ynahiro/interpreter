#ifndef RPN_H
#define RPN_H

#include <string>
#include <vector>

enum class OpCode {
    PUSH_INT,   // int_arg = значение
    LOAD,       // str_arg = имя переменной
    STORE,      // str_arg = имя переменной
    ASTORE,     // стек: [индекс, значение]; str_arg = имя массива
    ADD, SUB, MUL, DIV, MOD,
    EQ, NEQ, LT, GT, LEQ, GEQ,
    PRINTF,
    SCANF,      // str_arg = имя переменной
    LABEL,      // int_arg = id метки
    JIT,        // jump if true; int_arg = id метки
    DECL_VAR,   // str_arg = имя переменной
    DECL_ARR,   // str_arg = имя; int_arg = размер
};

struct RpnInstruction {
    OpCode op;
    std::string str_arg;
    int int_arg;

    RpnInstruction(OpCode op, std::string str = "", int i = 0)
        : op(op), str_arg(std::move(str)), int_arg(i) {}
};

void print_rpn(const std::vector<RpnInstruction>& code);

#endif
