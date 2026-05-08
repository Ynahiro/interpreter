#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "rpn.h"
#include <iostream>
#include <vector>
#include <string>

class Parser {
private:
    Lexer& lexer;
    size_t current_token_index;
    Token  current_token;
    int    label_counter;

    // Вспомогательные функции
    void advance();
    bool match(TokenType expected_type);
    bool check(TokenType expected_type);
    void error(const std::string& message);
    void emit(RpnInstruction instr);
    int  new_label();

    // Правила грамматики
    void program();
    void statement();
    void declaration();
    void assignment();
    void expression();
    void factor();
    void condition();
    void do_while_statement();
    void input_statement();
    void output_statement();
    void array_declaration(const std::string& arr_name);
    void array_access(const std::string& arr_name);

public:
    std::vector<RpnInstruction> rpn_code;

    Parser(Lexer& lexer);
    bool parse();
};

#endif
