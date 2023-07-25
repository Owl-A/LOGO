#ifndef __INTERPRET_H
#define __INTERPRET_H
#include "ast.h"
#include <queue>

class Interpret : public Visitor {
  std::queue<AST_Atomic *> *action_queue;

public:
  Interpret(std::queue<AST_Atomic *> *q) : action_queue(q) {}

  virtual void visitAtomic(AST_Atomic *ast) {
    action_queue->push(new AST_Atomic(new Token(*ast->command), ast->value));
  }

  virtual void visitBlock(AST_Block *ast) {
    for (std::vector<AST *>::iterator it = ast->block.begin();
         it != ast->block.end(); ++it) {
      (*it)->accept(this);
    }
  }

  virtual void visitRepeat(AST_Repeat *ast) {
    for (int i = 0; i < ast->times; i++) {
      ast->body->accept(this);
    }
  }

  void execute(AST *ast) { ast->accept(this); }
};

#endif
