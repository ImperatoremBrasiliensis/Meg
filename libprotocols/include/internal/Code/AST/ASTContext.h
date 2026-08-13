/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_AST_ASTCONTEXT_H
#define PROTOCOLS_INTERNAL_CODE_AST_ASTCONTEXT_H

#include <internal/Code/AST/AST.h>
#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

/* =============== AST Context =============== */

typedef struct prosASTContext_s {
   prosAST *ast;
   prosASTNode **ctxStack;
   size_t ctxStackSize, ctxDepth;
} prosASTContext;

prosASTContext prosASTContext_new(prosAST *ast);

void prosASTContext_del(prosASTContext *self);

bool prosASTContext_traverseDown(prosASTContext *self, prosASTNode *node);

bool prosASTContext_traverseUp(prosASTContext *self);

prosASTNode *prosASTContext_getCurrentNode(prosASTContext *self);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_CODE_AST_AST_H
