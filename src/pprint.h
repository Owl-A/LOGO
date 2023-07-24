#ifndef __PPRINT_H
#define __PPRINT_H
#include <vector>
#include "ast.h"

class PPrint : public Visitor{
  int indent;

  void pindent() {
    for ( int i = 0; i < indent; i++) {
      std::cout << "  ";
    }
  }
public:
  PPrint() : indent(0) {}

  virtual void visitAtomic(AST_Atomic* ast) {
    pindent();
    std::cout << "{ ATOMIC " << ast->command->lexeme << " " << ast->value << " }\n";
  }

  virtual void visitBlock(AST_Block* ast) {
    pindent();
    std::cout << "{ BLOCK\n";
    indent++;
    for ( std::vector<AST*>::iterator it = ast->block.begin();
	  it != ast->block.end(); ++it ) {
      (*it)->accept(this);
    }
    indent--;
    pindent();
    std::cout << "}\n";
  }

  virtual void visitRepeat(AST_Repeat* ast) {
    pindent();
    std::cout << "{ REPEAT " << ast->times << "\n";
    indent++;
    ast->body->accept(this);
    indent--;
    pindent();
    std::cout << "}\n";
  }

  void print(AST* ast) {
    ast->accept(this);
  }
};

PPrint pprinter;

#endif
