#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>

enum TokenType {
  TOKEN_INT = 1,
  TOKEN_DO,
  TOKEN_WHILE,
  TOKEN_SCANF,
  TOKEN_PRINTF,
  TOKEN_RETURN,

  TOKEN_ASSIGN,   // =
  TOKEN_PLUS,     // +
  TOKEN_MINUS,    // -
  TOKEN_MULTIPLY, // *
  TOKEN_DIVIDE,   // /
  TOKEN_MOD,      // %
  TOKEN_NOT,      // !

  TOKEN_EQUAL,         // ==
  TOKEN_NOT_EQUAL,     // !=
  TOKEN_LESS,          // <
  TOKEN_GREATER,       // >
  TOKEN_LESS_EQUAL,    // <=
  TOKEN_GREATER_EQUAL, // >=

  TOKEN_SEMICOLON, // ;
  TOKEN_COMMA,     // ,
  TOKEN_LPAREN,    // (
  TOKEN_RPAREN,    // )
  TOKEN_LBRACE,    // {
  TOKEN_RBRACE,    // }
  TOKEN_LBRACKET,  // [
  TOKEN_RBRACKET,  // ]

  TOKEN_IDENTIFIER,
  TOKEN_CONSTANT,
  TOKEN_EOF,
  TOKEN_UNKNOWN
};

struct Token {
  TokenType type;
  std::string value;
  int line;
  int position;

  Token(TokenType t, const std::string &v, int l, int p)
      : type(t), value(v), line(l), position(p) {}
};

struct Symbol {
  std::string name;
  std::string type;
  std::string value;
  int declared_line;
  int index;

  Symbol(const std::string &n, const std::string &t, const std::string &v, int l, int idx)
      : name(n), type(t), value(v), declared_line(l), index(idx) {}
};

struct Constant {
  std::string value;
  std::string type;
  int line;
  int index;

  Constant(const std::string &v, const std::string &t, int l, int idx)
      : value(v), type(t), line(l), index(idx) {}
};

struct Error {
  int line;
  int position;
  int index;

  Error(int l, int p, int idx) : line(l), position(p), index(idx) {}
};

struct Delimiter {
  Token token;
  int internal_code;
  int table_index;
  
  Delimiter(const Token &t, int code, int idx) 
      : token(t), internal_code(code), table_index(idx) {}
};

struct Keyword {
  Token token;
  int internal_code;
  int table_index;
  
  Keyword(const Token &t, int code, int idx) 
      : token(t), internal_code(code), table_index(idx) {}
};

struct TableInfo {
    int table_number;
    std::string table_name;
    std::string lexeme_type;
    std::string description;
};

class Lexer {
private:
  std::ifstream file;
  std::string filename;
  int line;
  int position;
  char current_char;
  
  std::map<std::string, TokenType> keywords;
  std::map<TokenType, std::string> token_names;
  std::vector<TableInfo> table_mapping;

  // Счетчики для индексов в таблицах
  int symbol_index;
  int constant_index;
  int error_index;
  int delimiter_index;
  int keyword_index;

  void next_char();
  void skip_whitespace();
  Token get_identifier();
  Token get_number();
  Token get_operator();
  void add_to_symbol_table(const std::string &name, const std::string &type,
                         const std::string &value, int line);
  void add_to_constant_table(const std::string &value, const std::string &type,
                           int line);
  void add_delimiter(Token token, int internal_code);

public:
  std::vector<Token> tokens;
  std::vector<Symbol> symbol_table;
  std::vector<Constant> constant_table;
  std::vector<Error> errors;
  std::vector<Delimiter> delimiters;
  std::vector<Keyword> keywords_found;

  Lexer(const std::string &filename);
  ~Lexer();

  std::string get_token_name(TokenType type);
  void analyze();
  void print_results();
  void print_table_references();
};

#endif
