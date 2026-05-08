#include "parser.h"
#include "lexer.h"

Parser::Parser(Lexer& lexer)
    : lexer(lexer), current_token_index(0), label_counter(0),
      current_token(lexer.tokens.empty() ? Token(TOKEN_EOF, "EOF", 0, 0)
                                         : lexer.tokens[0]) {}

void Parser::advance() {
    current_token_index++;
    if (current_token_index < lexer.tokens.size())
        current_token = lexer.tokens[current_token_index];
    else
        current_token = Token(TOKEN_EOF, "EOF", 0, 0);
}

bool Parser::match(TokenType expected_type) {
    if (current_token.type == expected_type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType expected_type) {
    return current_token.type == expected_type;
}

void Parser::error(const std::string& message) {
    std::cerr << "Синтаксическая ошибка на строке " << current_token.line
              << ", позиция " << current_token.position << ": " << message << std::endl;
    std::cerr << "Текущий токен: " << lexer.get_token_name(current_token.type)
              << " '" << current_token.value << "'" << std::endl;
    throw std::runtime_error("Синтаксическая ошибка");
}

void Parser::emit(RpnInstruction instr) {
    rpn_code.push_back(std::move(instr));
}

int Parser::new_label() {
    return label_counter++;
}

bool Parser::parse() {
    try {
        program();
        if (!check(TOKEN_EOF)) {
            error("Ожидался конец файла");
            return false;
        }
        std::cout << "Синтаксический анализ завершён успешно!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка парсинга: " << e.what() << std::endl;
        return false;
    }
}

// Z -> statement_list
void Parser::program() {
    while (!check(TOKEN_EOF))
        statement();
}

// statement -> declaration | assignment | do_while | scanf | printf
void Parser::statement() {
    if      (check(TOKEN_INT))        declaration();
    else if (check(TOKEN_IDENTIFIER)) assignment();
    else if (check(TOKEN_SCANF))      input_statement();
    else if (check(TOKEN_PRINTF))     output_statement();
    else if (check(TOKEN_DO))         do_while_statement();
    else                              error("Ожидался оператор");
}

// declaration -> "int" i ";" | "int" i "=" expression ";" | "int" i array_declaration
void Parser::declaration() {
    if (!match(TOKEN_INT))
        error("Ожидался 'int'");

    std::string name = current_token.value;
    if (!match(TOKEN_IDENTIFIER))
        error("Ожидался идентификатор");

    if (check(TOKEN_LBRACKET)) {
        array_declaration(name);
    } else {
        emit(RpnInstruction(OpCode::DECL_VAR, name));
        if (match(TOKEN_ASSIGN)) {
            expression();
            emit(RpnInstruction(OpCode::STORE, name));
        }
        if (!match(TOKEN_SEMICOLON))
            error("Ожидалось ';'");
    }
}

// assignment -> i "=" expression ";" | i array_access
void Parser::assignment() {
    std::string name = current_token.value;   // сохраняем до вызова match
    if (!match(TOKEN_IDENTIFIER))
        error("Ожидался идентификатор");

    if (check(TOKEN_LBRACKET)) {
        array_access(name);
    } else {
        if (!match(TOKEN_ASSIGN))
            error("Ожидалось '='");
        expression();
        emit(RpnInstruction(OpCode::STORE, name));
        if (!match(TOKEN_SEMICOLON))
            error("Ожидалось ';'");
    }
}

// expression -> factor { ("+"|"-"|"*"|"/"|"%") factor }
void Parser::expression() {
    factor();

    while (check(TOKEN_PLUS) || check(TOKEN_MINUS) ||
           check(TOKEN_MULTIPLY) || check(TOKEN_DIVIDE) || check(TOKEN_MOD)) {
        TokenType op = current_token.type;
        advance();
        factor();

        switch (op) {
            case TOKEN_PLUS:     emit(RpnInstruction(OpCode::ADD)); break;
            case TOKEN_MINUS:    emit(RpnInstruction(OpCode::SUB)); break;
            case TOKEN_MULTIPLY: emit(RpnInstruction(OpCode::MUL)); break;
            case TOKEN_DIVIDE:   emit(RpnInstruction(OpCode::DIV)); break;
            case TOKEN_MOD:      emit(RpnInstruction(OpCode::MOD)); break;
            default: break;
        }
    }
}

// factor -> i | constant | "(" expression ")"
void Parser::factor() {
    if (check(TOKEN_IDENTIFIER)) {
        std::string name = current_token.value;
        advance();
        emit(RpnInstruction(OpCode::LOAD, name));
    } else if (check(TOKEN_CONSTANT)) {
        int val = std::stoi(current_token.value);
        advance();
        emit(RpnInstruction(OpCode::PUSH_INT, "", val));
    } else if (match(TOKEN_LPAREN)) {           // исправлен баг: было !match
        expression();
        if (!match(TOKEN_RPAREN))
            error("Ожидалось ')'");
    } else {
        error("Ожидалось: идентификатор, константа или '('");
    }
}

// array_declaration -> "[" constant "]" ";"
void Parser::array_declaration(const std::string& arr_name) {
    if (!match(TOKEN_LBRACKET))
        error("Ожидалось '['");
    if (!check(TOKEN_CONSTANT))
        error("Ожидалась константа размера массива");
    int size = std::stoi(current_token.value);
    advance();
    if (!match(TOKEN_RBRACKET))
        error("Ожидалось ']'");
    if (!match(TOKEN_SEMICOLON))
        error("Ожидалось ';'");
    emit(RpnInstruction(OpCode::DECL_ARR, arr_name, size));
}

// array_access -> "[" factor "]" "=" expression ";"
// Стек перед ASTORE: [..., индекс, значение]
void Parser::array_access(const std::string& arr_name) {
    if (!match(TOKEN_LBRACKET))
        error("Ожидалось '['");
    factor();                   // эмитируем индекс
    if (!match(TOKEN_RBRACKET))
        error("Ожидалось ']'");
    if (!match(TOKEN_ASSIGN))
        error("Ожидалось '='");
    expression();               // эмитируем значение
    emit(RpnInstruction(OpCode::ASTORE, arr_name));
    if (!match(TOKEN_SEMICOLON))
        error("Ожидалось ';'");
}

// output_statement -> "printf" "(" expression ")" ";"
void Parser::output_statement() {
    if (!match(TOKEN_PRINTF))
        error("Ожидался 'printf'");
    if (!match(TOKEN_LPAREN))
        error("Ожидалось '('");
    expression();
    if (!match(TOKEN_RPAREN))
        error("Ожидалось ')'");
    if (!match(TOKEN_SEMICOLON))
        error("Ожидалось ';'");
    emit(RpnInstruction(OpCode::PRINTF));
}

// input_statement -> "scanf" "(" i ")" ";"
// Семантически scanf принимает только l-value (идентификатор переменной)
void Parser::input_statement() {
    if (!match(TOKEN_SCANF))
        error("Ожидался 'scanf'");
    if (!match(TOKEN_LPAREN))
        error("Ожидалось '('");
    std::string var_name = current_token.value;
    if (!match(TOKEN_IDENTIFIER))
        error("scanf ожидает идентификатор переменной");
    if (!match(TOKEN_RPAREN))
        error("Ожидалось ')'");
    if (!match(TOKEN_SEMICOLON))
        error("Ожидалось ';'");
    emit(RpnInstruction(OpCode::SCANF, var_name));
}

// do_while_statement -> "do" block "while" "(" condition ")" ";"
// Польская запись:
//   LABEL Ln
//   <тело>
//   <условие>
//   JIT Ln
void Parser::do_while_statement() {
    int loop_label = new_label();
    emit(RpnInstruction(OpCode::LABEL, "", loop_label));

    if (!match(TOKEN_DO))
        error("Ожидалось 'do'");
    if (!match(TOKEN_LBRACE))
        error("Ожидалось '{'");

    while (!check(TOKEN_RBRACE) && !check(TOKEN_EOF))
        statement();

    if (!match(TOKEN_RBRACE))
        error("Ожидалось '}'");
    if (!match(TOKEN_WHILE))
        error("Ожидалось 'while'");
    if (!match(TOKEN_LPAREN))
        error("Ожидалось '('");

    condition();

    if (!match(TOKEN_RPAREN))
        error("Ожидалось ')'");
    if (!match(TOKEN_SEMICOLON))
        error("Ожидалось ';'");

    emit(RpnInstruction(OpCode::JIT, "", loop_label));
}

// condition -> expression relop expression
void Parser::condition() {
    expression();

    TokenType relop = current_token.type;
    if (check(TOKEN_EQUAL) || check(TOKEN_NOT_EQUAL) ||
        check(TOKEN_LESS)  || check(TOKEN_GREATER)   ||
        check(TOKEN_LESS_EQUAL) || check(TOKEN_GREATER_EQUAL)) {
        advance();
        expression();

        switch (relop) {
            case TOKEN_EQUAL:         emit(RpnInstruction(OpCode::EQ));  break;
            case TOKEN_NOT_EQUAL:     emit(RpnInstruction(OpCode::NEQ)); break;
            case TOKEN_LESS:          emit(RpnInstruction(OpCode::LT));  break;
            case TOKEN_GREATER:       emit(RpnInstruction(OpCode::GT));  break;
            case TOKEN_LESS_EQUAL:    emit(RpnInstruction(OpCode::LEQ)); break;
            case TOKEN_GREATER_EQUAL: emit(RpnInstruction(OpCode::GEQ)); break;
            default: break;
        }
    } else {
        error("Ожидался оператор сравнения");
    }
}
