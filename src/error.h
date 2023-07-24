#ifndef __ERROR_H
#define __ERROR_H
#include "lexer.h"

class SyntaxError : public std::exception {
public:
  Token* token;
  std::string msg;

  SyntaxError(Token* token, std::string msg) : std::exception() {
    this->token = token;
    this->msg = msg;
  }
};

#endif
