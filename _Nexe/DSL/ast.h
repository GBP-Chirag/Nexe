/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/
#ifndef NEXE_DSL_AST_H
#define NEXE_DSL_AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AST AST; // Forward reference

// These nodes describe the public BUILD.sbl model. The C++ engine currently
// evaluates the built-in C/C++ rules directly; a future extension evaluator
// can use the same stable rule/toolchain boundary.
struct AST {
    enum {
        AST_BUILD,
    AST_TASK,
    AST_RUN,
  } tag;
  union {
    struct AST_BUILD {  number; } AST_NUMBER;
    struct AST_TASK { AST *left; AST *right; } AST_ADD;
    struct AST_RUN { AST *left; AST *right; } AST_MUL;
  } data;
};

 // namespace nexe::dsl

#endif // NEXE_DSL_AST_H
