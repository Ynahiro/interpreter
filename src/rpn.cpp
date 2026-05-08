#include "rpn.h"
#include <iostream>
#include <iomanip>

void print_rpn(const std::vector<RpnInstruction>& code) {
    std::cout << "=== ПОЛЬСКАЯ ЗАПИСЬ ===" << std::endl;
    std::cout << std::setw(4) << "№" << " | Инструкция" << std::endl;
    std::cout << std::string(35, '-') << std::endl;

    for (size_t i = 0; i < code.size(); ++i) {
        const auto& instr = code[i];
        std::cout << std::setw(4) << i << " | ";

        switch (instr.op) {
            case OpCode::PUSH_INT:  std::cout << instr.int_arg; break;
            case OpCode::LOAD:      std::cout << instr.str_arg; break;
            case OpCode::STORE:     std::cout << "STORE "    << instr.str_arg; break;
            case OpCode::ASTORE:    std::cout << "ASTORE "   << instr.str_arg; break;
            case OpCode::ADD:       std::cout << "+"; break;
            case OpCode::SUB:       std::cout << "-"; break;
            case OpCode::MUL:       std::cout << "*"; break;
            case OpCode::DIV:       std::cout << "/"; break;
            case OpCode::MOD:       std::cout << "%"; break;
            case OpCode::EQ:        std::cout << "=="; break;
            case OpCode::NEQ:       std::cout << "!="; break;
            case OpCode::LT:        std::cout << "<"; break;
            case OpCode::GT:        std::cout << ">"; break;
            case OpCode::LEQ:       std::cout << "<="; break;
            case OpCode::GEQ:       std::cout << ">="; break;
            case OpCode::PRINTF:    std::cout << "PRINTF"; break;
            case OpCode::SCANF:     std::cout << "SCANF "    << instr.str_arg; break;
            case OpCode::LABEL:     std::cout << "L" << instr.int_arg << ":"; break;
            case OpCode::JIT:       std::cout << "JIT L"     << instr.int_arg; break;
            case OpCode::DECL_VAR:  std::cout << "DECL "     << instr.str_arg; break;
            case OpCode::DECL_ARR:  std::cout << "DECL_ARR " << instr.str_arg
                                              << "[" << instr.int_arg << "]"; break;
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}
