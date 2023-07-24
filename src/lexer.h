#ifndef __LEXER_H
#define __LEXER_H

#include <iostream>
#include <cstring>
#include <sstream>
#include <istream>

enum TokenType {
  LEFT_BRACKET,
  RIGHT_BRACKET,

  FORWARD,
  BACKWARD,
  RIGHT,
  LEFT,

  INTEGER,
  REPEAT,
  IDENTIFIER,
  EOFF
};

class Token {
public:
  void* value;
  TokenType type;
  std::string lexeme;

  Token(void* value, TokenType type, std::string lexeme) {
    this->value = value;
    this->type = type;
    this->lexeme = lexeme;
  }

  void pprint() {
    std::cout << "{ TOKEN " << type << " " <<  lexeme << " }\n";
  }

  bool isEOF() {
    return type == EOFF;
  }
};

class Lexer{
  std::stringstream input;
  char current;

  bool isAlpha(char c) {
    return ('a' <= c && 'z' >= c) || 
	   ('A' <= c && 'Z' >= c);
  }

  bool isNumeric(char c) {
    return ('0' <= c && '9' >= c);
  }

  bool isAlphaNumeric(char c) {
    return isNumeric(c) || isAlpha(c);
  }

  Token* process(std::string lex) {
    if (strcmp("forward", lex.c_str()) == 0) return new Token(NULL, FORWARD, lex);
    if (strcmp("backward", lex.c_str()) == 0) return new Token(NULL, BACKWARD, lex);
    if (strcmp("right", lex.c_str()) == 0) return new Token(NULL, RIGHT, lex);
    if (strcmp("left", lex.c_str()) == 0) return new Token(NULL, LEFT, lex);
    if (strcmp("repeat", lex.c_str()) == 0) return new Token(NULL, REPEAT, lex);
    return new Token(NULL, IDENTIFIER, lex);
  }

  void advance() {
    input.get(current);
    if (input.eof()) current = '\0';
  }

public:
  Lexer(std::string in) : input(in) {
    input >> current;
  }
  
  Token* next() {
    if (current == '[') { 
      advance(); 
      return new Token(NULL, LEFT_BRACKET, "[");
    } else if (current == ']') { 
      advance(); 
      return new Token(NULL, RIGHT_BRACKET, "]");
    } else if (isAlpha(current)) {
      std::stringstream tmp;
      while(isAlphaNumeric(current)) { tmp.put(current); advance(); }
      std::string lex;
      tmp >> lex;
      return process(lex);
    } else if (isNumeric(current)) {
      std::stringstream tmp;
      while(isNumeric(current)) { tmp.put(current); advance(); }
      std::string lex;
      tmp >> lex;
      return new Token((void*)atoll(lex.c_str()), INTEGER, lex);
    } else if ( current == ' ' || current == '\t' || current == '\n' ) { 
      advance(); 
      return next();
    } else if (current == '\0') return new Token(NULL, EOFF, "");
    else throw std::exception();
  }
};

#endif
