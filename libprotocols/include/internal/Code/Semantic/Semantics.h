/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_SEMANTIC_SEMANTICS_H
#define PROTOCOLS_INTERNAL_CODE_SEMANTIC_SEMANTICS_H

#include <internal/Code/AST/ASTBuilder.h>
#include <internal/Code/Semantic/Scope.h>
#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef struct prosSemantics_s {
   prosVector scopeStack;
   prosAllocator allocator;
   prosASTBuilder builder;
   prosIdTable *idTable;
   prosASTNode *mainNode;
} prosSemantics;

prosSemantics prosSemantics_new(prosIdTable *idTable);

void prosSemantics_del(prosSemantics *self);

bool prosSemantics_initModuleScope(prosSemantics *self, prosString name);

bool prosSemantics_checkScopeBounds(prosSemantics *self, prosToken indent);

bool prosSemantics_actOnOperationScope(
   prosSemantics *self,
   prosToken dedent
);

bool prosSemantics_actOnEndOfFile(prosSemantics *self, prosToken eof);

prosScopeDeclType prosSemantics_getIdentifierInfo(
   prosSemantics *self,
   prosToken id
);

typedef struct prosSemanticsFuncParam_s {
   prosToken id;
   prosToken type;
} prosSemanticsFuncParam;

bool prosSemantics_actOnFuncDecl(
   prosSemantics *self,
   prosToken identifier,
   prosToken returnType,
   prosSemanticsFuncParam params[],
   size_t paramCount
);

typedef enum prosSemanticsExprBonsaiNodeType_e : uint8_t {
   PROS_SEMANTICS_EXPR_BONSAI_NODE_NONE,
   PROS_SEMANTICS_EXPR_BONSAI_NODE_DECL_REF,
   PROS_SEMANTICS_EXPR_BONSAI_NODE_NUMBER_LITERAL,
   PROS_SEMANTICS_EXPR_BONSAI_NODE_BOOL_LITERAL,
   PROS_SEMANTICS_EXPR_BONSAI_NODE_BIN_OP,
   PROS_SEMANTICS_EXPR_BONSAI_NODE_UNA_OP,
   PROS_SEMANTICS_EXPR_BONSAI_NODE_PAREN,
   PROS_SEMANTICS_EXPR_BONSAI_NODE_CALL_OP
} prosSemanticsExprBonsaiNodeType;

typedef struct prosSemanticsExprBonsaiNode_s {
   prosToken tok;

   union {
      struct {
         size_t operand1, operand2;
      } binOp;
      struct {
         size_t operand;
      } unaOp;
      size_t paren;
      struct {
         size_t funcRefIndex;
         prosVector args;
      } callOperator;
      struct {
         bool isTrue;
      } boolLiteral;
   } as;

   prosSemanticsExprBonsaiNodeType type;
} prosSemanticsExprBonsaiNode;

void prosSemanticsExprBonsaiNode_del(prosSemanticsExprBonsaiNode *self);

bool prosSemantics_actOnObjectDecl(
   prosSemantics *self,
   prosToken identifier,
   prosToken typeId,
   bool value,
   prosVector *initilizerBonsai
);

bool prosSemantics_actOnIfStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *exprBonsai
);

bool prosSemantics_actOnOrStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *exprBonsai
);

bool prosSemantics_actOnElseStatement(
   prosSemantics *self,
   prosToken stmt
);

bool prosSemantics_actOnWhileStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *exprBonsai
);

bool prosSemantics_actOnLoopStatement(
   prosSemantics *self,
   prosToken stmt
);

bool prosSemantics_actOnExpressionStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *bonsai
);

bool prosSemantics_actOnBreakStatement(
   prosSemantics *self,
   prosToken stmt
);

bool prosSemantics_actOnContinueStatement(
   prosSemantics *self,
   prosToken stmt
);

bool prosSemantics_actOnReturnStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *bonsai
);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_CODE_SEMANTIC_SEMANTICS_H
