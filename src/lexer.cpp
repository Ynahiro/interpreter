#include "lexer.h"
#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>

Lexer::Lexer(const std::string &filename)
    : filename(filename), line(1), position(0), symbol_index(0),
      constant_index(0), error_index(0), delimiter_index(0), keyword_index(0) {

  keywords = {{"int", TOKEN_INT},       {"do", TOKEN_DO},
              {"while", TOKEN_WHILE},   {"scanf", TOKEN_SCANF},
              {"printf", TOKEN_PRINTF}, {"return", TOKEN_RETURN}};

  token_names = {{TOKEN_SEMICOLON, "SEMICOLON"}, {TOKEN_COMMA, "COMMA"},
                 {TOKEN_LPAREN, "LPAREN"},       {TOKEN_RPAREN, "RPAREN"},
                 {TOKEN_LBRACE, "LBRACE"},       {TOKEN_RBRACE, "RBRACE"},
                 {TOKEN_LBRACKET, "LBRACKET"},   {TOKEN_RBRACKET, "RBRACKET"}};

  table_mapping = {{0, "symbolic_names", "IDENTIFIER", "Идентификаторы"},
                   {1, "constants", "CONSTANT", "Целые числа"},
                   {2, "keywords", "KEYWORD", "Ключевые слова"},
                   {3, "separators", "SEPARATOR", "Разделители и операторы"},
                   {4, "errors", "ERROR", "Ошибки"}};

  file.open(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + filename);
  }
  next_char(); // первый символ
}

Lexer::~Lexer() {
  if (file.is_open()) {
    file.close();
  }
}

void Lexer::next_char() {
  if (file.get(current_char)) {
    position++;
    if (current_char == '\n') {
      line++;
      position = 0;
    }
  } else {
    current_char = EOF;
  }
}

void Lexer::skip_whitespace() {
  while (isspace(current_char) && current_char != EOF) {
    next_char();
  }
}

Token Lexer::get_identifier() {
  int start_line = line;
  int start_pos = position;
  std::string result;

  while (isalnum(current_char) || current_char == '_') {
    result += current_char;
    next_char();
  }

  if (keywords.find(result) != keywords.end()) {
    Token token(keywords[result], result, start_line, start_pos);
    // Добавляем в таблицу ключевых слов
    bool found = false;
    for (const auto &kw : keywords_found) {
      if (kw.token.value == result) {
        found = true;
        break;
      }
    }
    if (!found) {
      keywords_found.emplace_back(token, static_cast<int>(keywords[result]),
                                  keyword_index++);
    }
    return token;
  }

  add_to_symbol_table(result, "unknown", "", start_line);

  return Token(TOKEN_IDENTIFIER, result, start_line, start_pos);
}

Token Lexer::get_number() {
  int start_line = line;
  int start_pos = position;
  std::string result;

  while (isdigit(current_char)) {
    result += current_char;
    next_char();
  }

  if (result[0] == '0' && result.length() > 1) {
    errors.emplace_back(start_line, start_pos, error_index++);
    result.erase(0, 1);
  }

  add_to_constant_table(result, "int", start_line);

  return Token(TOKEN_CONSTANT, result, start_line, start_pos);
}

Token Lexer::get_operator() {
  int start_line = line;
  int start_pos = position;
  char first_char = current_char;
  std::string result;
  result += first_char;

  next_char();

  if (first_char == '=' && current_char == '=') {
    result += current_char;
    next_char();
    Token token(TOKEN_EQUAL, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_EQUAL));
    return token;
  } else if (first_char == '!' && current_char == '=') {
    result += current_char;
    next_char();
    Token token(TOKEN_NOT_EQUAL, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_NOT_EQUAL));
    return token;
  } else if (first_char == '<' && current_char == '=') {
    result += current_char;
    next_char();
    Token token(TOKEN_LESS_EQUAL, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_LESS_EQUAL));
    return token;
  } else if (first_char == '>' && current_char == '=') {
    result += current_char;
    next_char();
    Token token(TOKEN_GREATER_EQUAL, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_GREATER_EQUAL));
    return token;
  }

  Token token(TOKEN_UNKNOWN, result, start_line, start_pos);
  switch (first_char) {
  case '=':
    token = Token(TOKEN_ASSIGN, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_ASSIGN));
    break;
  case '+':
    token = Token(TOKEN_PLUS, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_PLUS));
    break;
  case '-':
    token = Token(TOKEN_MINUS, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_MINUS));
    break;
  case '*':
    token = Token(TOKEN_MULTIPLY, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_MULTIPLY));
    break;
  case '/':
    token = Token(TOKEN_DIVIDE, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_DIVIDE));
    break;
  case '%':
    token = Token(TOKEN_MOD, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_MOD));
    break;
  case '<':
    token = Token(TOKEN_LESS, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_LESS));
    break;
  case '>':
    token = Token(TOKEN_GREATER, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_GREATER));
    break;
  case '!':
    token = Token(TOKEN_NOT, result, start_line, start_pos);
    add_delimiter(token, static_cast<int>(TOKEN_NOT));
    break;
  default:
    token = Token(TOKEN_UNKNOWN, result, start_line, start_pos);
    break;
  }

  return token;
}

void Lexer::add_to_symbol_table(const std::string &name,
                                const std::string &type,
                                const std::string &value, int line) {
  for (const auto &symbol : symbol_table) {
    if (symbol.name == name) {
      return;
    }
  }
  symbol_table.emplace_back(name, type, value, line, symbol_index++);
}

void Lexer::add_to_constant_table(const std::string &value,
                                  const std::string &type, int line) {
  for (const auto &constant : constant_table) {
    if (constant.value == value && constant.type == type) {
      return;
    }
  }
  constant_table.emplace_back(value, type, line, constant_index++);
}

void Lexer::add_delimiter(Token token, int internal_code) {
  for (const auto &delim : delimiters) {
    if (delim.token.type == token.type && delim.token.value == token.value) {
      return;
    }
  }
  delimiters.emplace_back(token, internal_code, delimiter_index++);
}

void Lexer::analyze() {
  while (current_char != EOF) {
    skip_whitespace();

    if (current_char == EOF)
      break;

    Token token = Token(TOKEN_UNKNOWN, "", line, position);

    if (isalpha(current_char) || current_char == '_') {
      token = get_identifier();
    } else if (isdigit(current_char)) {
      token = get_number();
    } else if (strchr("=+-*/<>!&|", current_char)) {
      token = get_operator();
    } else {
      // Одиночные символы
      std::string char_str(1, current_char);
      int start_line = line;
      int start_pos = position;

      switch (current_char) {
      case ';': {
        token = Token(TOKEN_SEMICOLON, char_str, start_line, start_pos);
        add_delimiter(token, static_cast<int>(TOKEN_SEMICOLON));
        break;
      }
      case ',': {
        token = Token(TOKEN_COMMA, char_str, start_line, start_pos);
        add_delimiter(token, static_cast<int>(TOKEN_COMMA));
        break;
      }
      case '(': {
        token = Token(TOKEN_LPAREN, char_str, start_line, start_pos);
        add_delimiter(token, static_cast<int>(TOKEN_LPAREN));
        break;
      }
      case ')': {
        token = Token(TOKEN_RPAREN, char_str, start_line, start_pos);
        add_delimiter(token, static_cast<int>(TOKEN_RPAREN));
        break;
      }
      case '{': {
        token = Token(TOKEN_LBRACE, char_str, start_line, start_pos);
        add_delimiter(token, static_cast<int>(TOKEN_LBRACE));
        break;
      }
      case '}': {
        token = Token(TOKEN_RBRACE, char_str, start_line, start_pos);
        add_delimiter(token, static_cast<int>(TOKEN_RBRACE));
        break;
      }
      case '[': {
        token = Token(TOKEN_LBRACKET, char_str, start_line, start_pos);
        add_delimiter(token, static_cast<int>(TOKEN_LBRACKET));
        break;
      }
      case ']': {
        token = Token(TOKEN_RBRACKET, char_str, start_line, start_pos);
        add_delimiter(token, static_cast<int>(TOKEN_RBRACKET));
        break;
      }
      default:
        token = Token(TOKEN_UNKNOWN, char_str, start_line, start_pos);
        errors.emplace_back(start_line, start_pos, error_index++);
        break;
      }
      next_char();
    }

    tokens.push_back(token);
  }

  tokens.emplace_back(TOKEN_EOF, "EOF", line, position);
}

void Lexer::print_table_references() {
  std::cout << "ПРЕДСТАВЛЕНИЕ ДАННЫХ В ВИДЕ (ТАБЛИЦА, ИНДЕКС):" << std::endl;
  std::cout << "Формат: (номер_таблицы, индекс_в_таблице)" << std::endl;
  std::cout << "где: 0 - идентификаторы, 1 - константы, 2 - ключевые слова, 3 "
               "- разделители, 4 - ошибки"
            << std::endl;
  std::cout << std::string(50, '-') << std::endl;

  for (const auto &token : tokens) {
    if (token.type == TOKEN_EOF) {
      std::cout << "EOF" << std::endl;
      break;
    }

    int table_num = -1;
    int index = -1;

    switch (token.type) {
    case TOKEN_IDENTIFIER:
      table_num = 0;
      for (const auto &sym : symbol_table) {
        if (sym.name == token.value) {
          index = sym.index;
          break;
        }
      }
      break;
    case TOKEN_CONSTANT:
      table_num = 1;
      for (const auto &con : constant_table) {
        if (con.value == token.value) {
          index = con.index;
          break;
        }
      }
      break;
    case TOKEN_INT:
    case TOKEN_DO:
    case TOKEN_WHILE:
    case TOKEN_SCANF:
    case TOKEN_PRINTF:
      table_num = 2;
      for (const auto &kw : keywords_found) {
        if (kw.token.value == token.value) {
          index = kw.table_index;
          break;
        }
      }
      break;
    default:
      table_num = 3;
      for (const auto &delim : delimiters) {
        if (delim.token.value == token.value) {
          index = delim.table_index;
          break;
        }
      }
      break;
    }

    if (table_num != -1 && index != -1) {
      std::cout << "(" << table_num << ", " << index << ") ";
      std::cout << "[" << token.value << "] ";

      static int count = 0;
      if (++count % 5 == 0) {
        std::cout << std::endl;
      }
    }
  }
  std::cout << std::endl << std::endl;
}

void Lexer::print_results() {
  std::cout << "=== ЛЕКСИЧЕСКИЙ АНАЛИЗ ===" << std::endl;
  std::cout << "Файл: " << filename << std::endl << std::endl;

  std::cout << "ТАБЛИЦА СООТВЕТСТВИЯ ТАБЛИЦ И ТИПОВ ЛЕКСЕМ:" << std::endl;
  std::cout << std::setw(8) << "Номер" << std::setw(30) << "Имя таблицы"
            << std::setw(28) << "Тип лексем" << std::setw(28) << "Описание"
            << std::endl;
  std::cout << std::string(70, '-') << std::endl;

  for (const auto &table : table_mapping) {
    std::cout << std::setw(8) << table.table_number << std::setw(20)
              << table.table_name << std::setw(15) << table.lexeme_type
              << std::setw(35) << table.description << std::endl;
  }
  std::cout << std::endl;

  // Вывод представления данных
  print_table_references();

  std::cout << "ТАБЛИЦА ОШИБОК:" << std::endl;
  if (errors.empty()) {
    std::cout << "Ошибок не обнаружено" << std::endl;
  } else {
    std::cout << std::setw(8) << "Индекс" << std::setw(20) << "Строка"
              << std::setw(22) << "Позиция" << std::endl;
    std::cout << std::string(30, '-') << std::endl;
    for (const auto &error : errors) {
      std::cout << std::setw(8) << error.index << std::setw(8) << error.line
                << std::setw(12) << error.position << std::endl;
    }
  }
  std::cout << std::endl;

  std::cout << "ТАБЛИЦА РАЗДЕЛИТЕЛЕЙ:" << std::endl;
  if (delimiters.empty()) {
    std::cout << "Разделители не найдены" << std::endl;
  } else {
    std::cout << std::setw(8) << "Индекс" << std::setw(28) << "Внутр. код"
              << std::setw(14) << "Строка" << std::setw(16) << "Позиция"
              << std::setw(8) << "Тип" << std::setw(25) << "Значение"
              << std::endl;
    std::cout << std::string(65, '-') << std::endl;
    for (const auto &delim : delimiters) {
      std::cout << std::setw(8) << delim.table_index << std::setw(15)
                << delim.internal_code << std::setw(8) << delim.token.line
                << std::setw(8) << delim.token.position << std::setw(15)
                << get_token_name(delim.token.type) << std::setw(10)
                << delim.token.value << std::endl;
    }
  }
  std::cout << std::endl;

  std::cout << "ТАБЛИЦА КЛЮЧЕВЫХ СЛОВ:" << std::endl;
  if (keywords_found.empty()) {
    std::cout << "Ключевые слова не найдены" << std::endl;
  } else {
    std::cout << std::setw(8) << "Индекс" << std::setw(28) << "Внутр. код"
              << std::setw(14) << "Строка" << std::setw(16) << "Позиция"
              << std::setw(14) << "Тип" << std::setw(19) << "Значение"
              << std::endl;
    std::cout << std::string(65, '-') << std::endl;
    for (const auto &kw : keywords_found) {
      std::cout << std::setw(8) << kw.table_index << std::setw(15)
                << kw.internal_code << std::setw(8) << kw.token.line
                << std::setw(8) << kw.token.position << std::setw(15)
                << get_token_name(kw.token.type) << std::setw(10)
                << kw.token.value << std::endl;
    }
  }
  std::cout << std::endl;

  std::cout << "ТАБЛИЦА СИМВОЛИЧЕСКИХ ИМЕН:" << std::endl;
  std::cout << std::setw(8) << "Индекс" << std::setw(20) << "Имя"
            << std::setw(10) << "Тип" << std::setw(25) << "Значение"
            << std::setw(20) << "Строка" << std::endl;
  std::cout << std::string(60, '-') << std::endl;

  for (const auto &symbol : symbol_table) {
    std::cout << std::setw(8) << symbol.index << std::setw(15) << symbol.name
              << std::setw(10) << symbol.type << std::setw(15) << symbol.value
              << std::setw(10) << symbol.declared_line << std::endl;
  }
  std::cout << std::endl;

  std::cout << "ТАБЛИЦА КОНСТАНТ:" << std::endl;
  std::cout << std::setw(8) << "Индекс" << std::setw(28) << "Значение"
            << std::setw(10) << "Тип" << std::setw(20) << "Строка" << std::endl;
  std::cout << std::string(45, '-') << std::endl;

  for (const auto &constant : constant_table) {
    std::cout << std::setw(8) << constant.index << std::setw(15)
              << constant.value << std::setw(10) << constant.type
              << std::setw(10) << constant.line << std::endl;
  }
}

std::string Lexer::get_token_name(TokenType type) {
  switch (type) {
  case TOKEN_INT:
    return "INT";
  case TOKEN_DO:
    return "DO";
  case TOKEN_WHILE:
    return "WHILE";
  case TOKEN_SCANF:
    return "SCANF";
  case TOKEN_PRINTF:
    return "PRINTF";
  case TOKEN_IDENTIFIER:
    return "IDENTIFIER";
  case TOKEN_CONSTANT:
    return "CONSTANT";
  case TOKEN_ASSIGN:
    return "ASSIGN";
  case TOKEN_PLUS:
    return "PLUS";
  case TOKEN_MINUS:
    return "MINUS";
  case TOKEN_MULTIPLY:
    return "MULTIPLY";
  case TOKEN_DIVIDE:
    return "DIVIDE";
  case TOKEN_EQUAL:
    return "EQUAL";
  case TOKEN_NOT_EQUAL:
    return "NOT_EQUAL";
  case TOKEN_LESS:
    return "LESS";
  case TOKEN_GREATER:
    return "GREATER";
  case TOKEN_LESS_EQUAL:
    return "LESS_EQUAL";
  case TOKEN_GREATER_EQUAL:
    return "GREATER_EQUAL";
  case TOKEN_NOT:
    return "NOT";
  case TOKEN_SEMICOLON:
    return "SEMICOLON";
  case TOKEN_COMMA:
    return "COMMA";
  case TOKEN_LPAREN:
    return "LPAREN";
  case TOKEN_RPAREN:
    return "RPAREN";
  case TOKEN_LBRACE:
    return "LBRACE";
  case TOKEN_RBRACE:
    return "RBRACE";
  case TOKEN_LBRACKET:
    return "LBRACKET";
  case TOKEN_RBRACKET:
    return "RBRACKET";
  case TOKEN_EOF:
    return "EOF";
  default:
    return "UNKNOWN";
  }
}
