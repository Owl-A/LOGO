#ifndef __PARSER_H
#define __PARSER_H
#include <vector>
#include "lexer.h"
#include "error.h"
#include "ast.h"

class Parser{
  Lexer lexer;
  Token* current;
  
  AST* block() {
    std::vector<AST*> ret;
    while(current->type != RIGHT_BRACKET) ret.push_back(statement());
    current = lexer.next();
    return (AST*) new AST_Block(ret);
  }

  AST* repeat() {
    Token* repeat = current;
    Token* times = lexer.next();
    if (times->type != INTEGER) {
      throw SyntaxError(times, "Expect integer value after repeat");
    }
    current = lexer.next();
    AST* body = statement();
    return (AST*) new AST_Repeat(repeat, (long long int) times->value, body);
  }

  AST* statement() {
    if (current->type == FORWARD ||
        current->type == BACKWARD ||
        current->type == RIGHT ||
        current->type == LEFT) {
      Token* val = lexer.next();
      if (val->type != INTEGER) 
        throw SyntaxError(val, "Expect integer value after " + current->lexeme);
      AST* ret = (AST*) new AST_Atomic(current, (long long int) val->value);
      current = lexer.next();
      return ret;
    } else if (current->type == LEFT_BRACKET) {
      current = lexer.next();
      return block(); 
    } else if (current->type == REPEAT) {
      return repeat();
    }
    throw SyntaxError(current, "unexpected symbol");
  }

public:
  Parser(std::string buf) : lexer(buf) {
    current = lexer.next();
  }

  std::vector<AST*> parse() {
    std::vector<AST*> ret;
    while(current->type != EOFF){
      ret.push_back(statement());
    }
    return ret;
  }
};

#endif
