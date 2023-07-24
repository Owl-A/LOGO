#ifndef __AST_H
#define __AST_H
#include "lexer.h"

class AST;
class AST_Atomic;
class AST_Block;
class AST_Repeat;

class Visitor{
public:
  virtual void visitAtomic(AST_Atomic* ast) = 0;
  virtual void visitBlock(AST_Block* ast) = 0;
  virtual void visitRepeat(AST_Repeat* ast) = 0;
};

class AST{
public:
  virtual void accept(Visitor* visitor) = 0; 
};

class AST_Atomic{
public:
  Token* command;
  long long int value;
  AST_Atomic(Token* command, long long int value) {
    this->command = command;
    this->value = value;
  }

  ~AST_Atomic() {
    delete command;
  }

  virtual void accept(Visitor* visitor) {
    visitor->visitAtomic(this);
  }
};

class AST_Block{
public:
  std::vector<AST*> block;
  AST_Block(std::vector<AST*> block) {
    this->block = block;
  }

  ~AST_Block(){
    for (std::vector<AST*>::iterator it = block.begin() ; it != block.end() ; it++) {
      delete *it;
    }
  }

  virtual void accept(Visitor* visitor) {
    visitor->visitBlock(this);
  }
};

class AST_Repeat{
public:
  Token* repeat;
  long long int times;
  AST* body;

  AST_Repeat(Token* repeat, long long int times, AST* body) {
    this->repeat = repeat;
    this->times = times;
    this->body = body;
  }

  ~AST_Repeat() {
    delete this->repeat;
    delete this->body;
  }

  virtual void accept(Visitor* visitor) {
    visitor->visitRepeat(this);
  }
};


#endif
