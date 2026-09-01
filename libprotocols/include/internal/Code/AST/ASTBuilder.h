/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_AST_ASTBUILDER_H
#define PROTOCOLS_INTERNAL_CODE_AST_ASTBUILDER_H

#include <internal/Code/AST/AST.h>
#include <internal/Code/AST/ASTContext.h>
#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef struct prosASTBuilder_s {
   prosAST ast;
   prosASTContext ctx;
} prosASTBuilder;

prosASTBuilder prosASTBuilder_new(prosAllocator *allocator);

void prosASTBuilder_del(prosASTBuilder *self);

[[nodiscard]]
prosASTNode *prosASTBuilder_addNodeHandleNode(prosASTBuilder *self);

[[nodiscard]]
prosASTNode *prosASTBuilder_addModuleNode(
   prosASTBuilder *self,
   prosString name
);

[[nodiscard]]
prosASTNode *prosASTBuilder_addFuncDecl(
   prosASTBuilder *self,
   prosId *name,
   prosASTType type
);

prosASTNode *prosASTBuilder_addOperationNode(prosASTBuilder *self);

[[nodiscard]]
prosASTNode *prosASTBuilder_addFuncParameterDecl(
   prosASTBuilder *self,
   prosId *name,
   prosASTType type,
   size_t paramIndex
);

[[nodiscard]]
prosASTNode *prosASTBuilder_addObjectDecl(
   prosASTBuilder *self,
   prosId *name,
   prosASTType type,
   bool value
);

[[nodiscard]]
prosASTNode *prosASTBuilder_addIfStatementNode(prosASTBuilder *self);

[[nodiscard]]
prosASTNode *prosASTBuilder_addWhileStatementNode(prosASTBuilder *self);

[[nodiscard]]
prosASTNode *prosASTBuilder_addLoopStatementNode(prosASTBuilder *self);

[[nodiscard]]
prosASTNode *prosASTBuilder_addExpressionStatementNode(prosASTBuilder *self);

prosASTNode *prosASTBuilder_addBreakStatementNode(
   prosASTBuilder *self,
   prosASTNode *loop
);

prosASTNode *prosASTBuilder_addContinueStatementNode(
   prosASTBuilder *self,
   prosASTNode *loop
);

prosASTNode *prosASTBuilder_addReturnStatementNode(prosASTBuilder *self);

prosASTNode *prosASTBuilder_addExpression(
   prosASTBuilder *self,
   prosASTType type
);

prosASTNode *prosASTBuilder_addDeclRefExpr(
   prosASTBuilder *self,
   prosASTNode *objDecl,
   enum prosASTDeclRefType refType
);

prosASTNode *prosASTBuilder_addParensExprNode(
   prosASTBuilder *self,
   prosASTType type
);

prosASTNode *prosASTBuilder_addBinaryOperatorExprNode(
   prosASTBuilder *self,
   enum prosASTBinOpType binOpType
);

prosASTNode *prosASTBuilder_addUnaryOperatorExprNode(
   prosASTBuilder *self,
   enum prosASTUnaOpType unaOpType
);

prosASTNode *prosASTBuilder_addLiteralExprNode(
   prosASTBuilder *self,
   prosASTType type,
   uint32_t value,
   bool isSigned
);

prosASTNode *prosASTBuilder_addCallOperatorExprNode(prosASTBuilder *self);

[[nodiscard]]
prosASTNode *prosASTBuilder_addCallArgumentExpression(
   prosASTBuilder *self,
   prosASTType type
);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_CODE_AST_ASTBUILDER_H
