#include "src/interpreter.h"
#include "src/lexer.h"
#include "src/parser.h"
#include "src/rpn.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Использование: " << argv[0] << " <файл>" << std::endl;
    return 1;
  }

  try {
    Lexer lexer(argv[1]);
    lexer.analyze();
    lexer.print_results();

    Parser parser(lexer);
    if (!parser.parse()) {
      std::cerr << "Синтаксический анализ не пройден." << std::endl;
      return 1;
    }
    std::cout << std::endl;

    print_rpn(parser.rpn_code);

    Interpreter interpreter;
    interpreter.execute(parser.rpn_code);

  } catch (const std::exception &e) {
    std::cerr << "Ошибка: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
