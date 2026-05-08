#include "interpreter.h"
#include <iostream>
#include <stdexcept>

void Interpreter::push(int val) {
    stack.push_back(val);
}

int Interpreter::pop() {
    if (stack.empty())
        throw std::runtime_error("Стек пуст при исполнении инструкции");
    int val = stack.back();
    stack.pop_back();
    return val;
}

void Interpreter::execute(const std::vector<RpnInstruction>& code) {
    // Первый проход: строим карту меток label_id -> индекс инструкции
    std::map<int, size_t> labels;
    for (size_t i = 0; i < code.size(); ++i) {
        if (code[i].op == OpCode::LABEL)
            labels[code[i].int_arg] = i;
    }

    std::cout << "=== ВЫПОЛНЕНИЕ ПРОГРАММЫ ===" << std::endl;

    size_t ip = 0;
    while (ip < code.size()) {
        const RpnInstruction& ins = code[ip];

        switch (ins.op) {

            case OpCode::PUSH_INT:
                push(ins.int_arg);
                ++ip;
                break;

            case OpCode::LOAD:
                // Необъявленная переменная считается равной 0
                push(vars[ins.str_arg]);
                ++ip;
                break;

            case OpCode::STORE:
                vars[ins.str_arg] = pop();
                ++ip;
                break;

            case OpCode::ASTORE: {
                int val = pop();
                int idx = pop();
                auto it = arrays.find(ins.str_arg);
                if (it == arrays.end())
                    throw std::runtime_error("Массив не объявлен: " + ins.str_arg);
                if (idx < 0 || idx >= static_cast<int>(it->second.size()))
                    throw std::runtime_error(
                        "Индекс " + std::to_string(idx) +
                        " за пределами массива '" + ins.str_arg + "'");
                it->second[idx] = val;
                ++ip;
                break;
            }

            case OpCode::ADD: { int b = pop(), a = pop(); push(a + b); ++ip; break; }
            case OpCode::SUB: { int b = pop(), a = pop(); push(a - b); ++ip; break; }
            case OpCode::MUL: { int b = pop(), a = pop(); push(a * b); ++ip; break; }
            case OpCode::DIV: {
                int b = pop(), a = pop();
                if (b == 0) throw std::runtime_error("Деление на ноль");
                push(a / b); ++ip; break;
            }
            case OpCode::MOD: {
                int b = pop(), a = pop();
                if (b == 0) throw std::runtime_error("Деление на ноль");
                push(a % b); ++ip; break;
            }

            case OpCode::EQ:  { int b = pop(), a = pop(); push(a == b ? 1 : 0); ++ip; break; }
            case OpCode::NEQ: { int b = pop(), a = pop(); push(a != b ? 1 : 0); ++ip; break; }
            case OpCode::LT:  { int b = pop(), a = pop(); push(a <  b ? 1 : 0); ++ip; break; }
            case OpCode::GT:  { int b = pop(), a = pop(); push(a >  b ? 1 : 0); ++ip; break; }
            case OpCode::LEQ: { int b = pop(), a = pop(); push(a <= b ? 1 : 0); ++ip; break; }
            case OpCode::GEQ: { int b = pop(), a = pop(); push(a >= b ? 1 : 0); ++ip; break; }

            case OpCode::PRINTF:
                std::cout << ">> " << pop() << std::endl;
                ++ip;
                break;

            case OpCode::SCANF: {
                int val;
                std::cout << "<< введите " << ins.str_arg << ": ";
                std::cin >> val;
                vars[ins.str_arg] = val;
                ++ip;
                break;
            }

            case OpCode::LABEL:
                ++ip;
                break;

            case OpCode::JIT: {
                int cond = pop();
                if (cond) {
                    auto it = labels.find(ins.int_arg);
                    if (it == labels.end())
                        throw std::runtime_error(
                            "Метка не найдена: L" + std::to_string(ins.int_arg));
                    ip = it->second;
                } else {
                    ++ip;
                }
                break;
            }

            case OpCode::DECL_VAR:
                // Не перезаписываем уже объявленную переменную (повторный вход в цикл)
                if (vars.find(ins.str_arg) == vars.end())
                    vars[ins.str_arg] = 0;
                ++ip;
                break;

            case OpCode::DECL_ARR:
                if (arrays.find(ins.str_arg) == arrays.end())
                    arrays[ins.str_arg] = std::vector<int>(ins.int_arg, 0);
                ++ip;
                break;
        }
    }
}
