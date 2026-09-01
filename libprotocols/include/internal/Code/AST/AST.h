/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_AST_AST_H
#define PROTOCOLS_INTERNAL_CODE_AST_AST_H

#include <internal/Code/Lexicon/IdTable.h>
#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef uintptr_t prosASTType;

typedef enum prosASTNodeType_e : uint8_t {
   /* May be used to ignore AST branches. */
   PROS_AST_INVALID_NODE,
   /* Node pointer handle. */
   PROS_AST_NODE_HANDLE,
   /* Declarations. */
   PROS_AST_DECL_FUNC,
   PROS_AST_DECL_VAL,
   PROS_AST_DECL_VAR,
   PROS_AST_DECL_PARAMETER,
   /* Statements. */
   PROS_AST_IF_STATEMENT,
   PROS_AST_WHILE_STATEMENT,
   PROS_AST_FOR_STATEMENT,
   PROS_AST_REPEAT_STATEMENT,
   PROS_AST_LOOP_STATEMENT,
   PROS_AST_EXPRESSION_STATEMENT,
   PROS_AST_BREAK_STATEMENT,
   PROS_AST_CONTINUE_STATEMENT,
   PROS_AST_RETURN_STATEMENT,
   /* Expressions. */
   PROS_AST_EXPRESSION,
   PROS_AST_CALL_ARGUMENT_EXPRESSION,
   /* Expression Unities. */
   PROS_AST_EXPR_PARENS,
   PROS_AST_EXPR_LITERAL,
   PROS_AST_EXPR_BINARY_OPERATOR,
   PROS_AST_EXPR_UNARY_OPERATOR,
   PROS_AST_EXPR_DECL_REF,
   PROS_AST_EXPR_CALL_OP,
   /* Scopes. */
   PROS_AST_SCOPE_MODULE,
   PROS_AST_SCOPE_OPERATION
} prosASTNodeType;

typedef struct prosASTNode_s {
   // Node type.
   prosASTNodeType type;

   union prosASTNodeAs {
      struct prosASTNodeHandle {
         size_t size;
         struct prosASTCiclicNode_s *nodes;
      } nodeHandle;

      /* -- Declarations -- */
      struct prosASTFuncDecl {
         prosASTType retType;
         struct prosASTNode_s *params /* struct prosASTNodeHandler */;
         struct prosASTNode_s *body /* struct prosASToperation */;
         prosId *identifier;
      } funcDecl;
      struct prosASTParamDecl {
         prosASTType type;
         prosId *identifier;
      } funcParamDecl;
      struct prosASTValDecl {
         prosASTType type;
         struct prosASTNode_s *value /* struct prosASTExpression */;
         prosId *identifier;
      } valDecl;
      struct prosASTVarDecl {
         prosASTType type;
         struct prosASTNode_s *initializer /* struct prosASTExpression */;
         prosId *identifier;
      } varDecl;

      /* -- Statements -- */
      struct prosASTIfStatement {
         struct prosASTNode_s *expression /* struct prosASTExpression */;
         struct prosASTNode_s *then /* struct prosASTOperation */;

         /*
          * Cases of if type for `elseBlock` node>
          * PROS_AST_IF_STATEMENT_NONE -> nullptr (error);
          * PROS_AST_IF_STATEMENT_IF -> nullptr;
          * PROS_AST_IF_STATEMENT_IF_OR -> struct prosASTIfStatement;
          * PROS_AST_IF_STATEMENT_IF_ELSE -> struct prosASTOperation.
          */
         struct prosASTNode_s *elseBlock;
         enum prosASTIfStatementType {
            PROS_AST_IF_STATEMENT_NONE,
            PROS_AST_IF_STATEMENT_IF,
            PROS_AST_IF_STATEMENT_IF_OR,
            PROS_AST_IF_STATEMENT_IF_ELSE
         } ifType;
      } ifStatement;
      struct prosASTWhileStatement {
         struct prosASTNode_s *expression /* struct prosASTExpression */;
         struct prosASTNode_s *doBlock /* struct prosASTOperation */;
      } whileStatement;
      struct prosASTLoopStatement {
         struct prosASTNode_s *block /* struct prosASTOperation */;
      } loopStatement;
      struct prosASTExpressionStatement {
         prosASTType type;
         struct prosASTNode_s *child;
      } expressionStatement;
      struct prosASTBreakStatement {
         struct prosASTNode_s *loop;
      } breakStatement;
      struct proASTContinueStatement {
         struct prosASTNode_s *loop;
      } continueStatement;
      struct prosASTReturnStatement {
         struct prosASTNode_s *expression /* struct prosASTExpression */;
      } returnStatement;

      /* -- Expression -- */
      struct prosASTExpression {
         prosASTType type;
         struct prosASTNode_s *child;
      } expression;
      struct prosASTCallArgumentExpression {
         prosASTType type;
         struct prosASTNode_s *child; /* prosASTNodeHandle */
      } callArgumentExpression;
      struct prosASTParensExpr {
         prosASTType type;
         struct prosASTNode_s *childExpr;
      } parensExpr;
      struct prosASTLiteralExpr {
         prosASTType type;
         uint32_t intialValue;
         bool isSigned;
      } literalExpr;
      struct prosASTBinOpExpr {
         prosASTType type;
         struct prosASTNode_s *operand1, *operand2;
         enum prosASTBinOpType {
            PROS_AST_BIN_OP_NONE,
            PROS_AST_BIN_OP_EQUAL,
            PROS_AST_BIN_OP_PLUS,
            PROS_AST_BIN_OP_MINUS,
            PROS_AST_BIN_OP_STAR,
            PROS_AST_BIN_OP_SLASH,
            PROS_AST_BIN_OP_PLUSEQUAL,
            PROS_AST_BIN_OP_MINUSEQUAL,
            PROS_AST_BIN_OP_STAREQUAL,
            PROS_AST_BIN_OP_SLASHEQUAL,
         } binOpType;
      } binOpExpr;
      struct prosASTUnaOpExpr {
         prosASTType type;
         struct prosASTNode_s *operand;
         enum prosASTUnaOpType {
            PROS_AST_UNA_OP_NONE,
            PROS_AST_UNA_OP_PLUS,
            PROS_AST_UNA_OP_MINUS,
            PROS_AST_UNA_OP_EXCLAMATION
         } unaOpType;
      } unaOpExpr;
      struct prosASTDeclRefExpr {
         struct prosASTNode_s *decl;
         enum prosASTDeclRefType {
            PROS_AST_DECLREF_NONE,
            PROS_AST_DECLREF_FUNC,
            PROS_AST_DECLREF_FUNC_PARAM,
            PROS_AST_DECLREF_VAL,
            PROS_AST_DECLREF_VAR
         } declRefType;
      } declRefExpr;
      struct prosASTCallOpExpr {
         prosASTType type;
         struct prosASTNode_s *funcRef /* struct prosASTDeclRefExpr */;
         struct prosASTNode_s **arguments /* struct prosASTCallArgumentExpression */;
         size_t argCount;
      } callOpExpr;

      /* -- Scopes -- */
      struct prosASTModule {
         prosString name;
         struct prosASTCiclicNode_s *decls;
         uint32_t declCount;
      } module;
      struct prosASTOperation {
         struct prosASTCiclicNode_s *body;
         uint32_t instrCount;
      } operation;
   } nodeAs;
} prosASTNode;

/*
 * The last node in a ciclic list should
 * point to the last node that will be
 * the entry point.
 *
 * list handle -> last node -> 1o node.
 */
typedef struct prosASTCiclicNode_s {
   struct prosASTCiclicNode_s *next;
   prosASTNode node;
} prosASTCiclicNode;

typedef struct prosASTBuiltinType_s {
   const char name[16];
   size_t memorySize;
   bool sign;
} prosASTBuiltinType;

extern const prosASTBuiltinType PROS_VOID_TYPE;
extern const prosASTBuiltinType PROS_INT_TYPE;
extern const prosASTBuiltinType PROS_BOOL_TYPE;
extern const prosASTBuiltinType PROS_FLOAT_TYPE;
extern const prosASTBuiltinType PROS_CHAR_TYPE;

static constexpr size_t PROS_CONST_QUALIFIER_FLAG = 0b001;
static constexpr size_t PROS_SIGNED_TYPES_FLAG = 0b000100;

struct prosASTTypeDesc {
   union {
      prosASTNode *nodep;
      prosASTBuiltinType *builtinp;
   } as;
   bool isBuiltin;
   bool isConst;
   bool isSigned;
} prosAST_getTypeDecl(prosASTType ptr);

typedef struct prosAST_s {
   prosArena tree;
   prosASTNode *firstNode;
   size_t nodeCount;
} prosAST;

prosAST prosAST_new(prosAllocator *allocator);

void prosAST_del(prosAST *self);

prosASTNode *prosAST_addNode(prosAST *self, prosASTNode node);

prosASTNode *prosAST_addCiclicNode(prosAST *self, prosASTCiclicNode **last, prosASTNode node);

typedef struct prosASTListNavigator_s {
   prosASTCiclicNode *node1, *node;
} prosASTListNavigator;

[[maybe_unused]]
static PROS_INLINE prosASTListNavigator prosASTListNavigator_new(
   prosASTCiclicNode *list
) {
   return (prosASTListNavigator) {
      .node1 = list
   };
}

[[maybe_unused]]
static PROS_INLINE prosASTNode *prosASTListNavigator_next(
   prosASTListNavigator *self
) {
   if (self->node) {
      if (self->node != self->node1) {
         self->node = self->node->next;
         return &self->node->node;
      }
   } else {
      self->node = self->node1->next;
      return &self->node->node;
   }

   return nullptr;
}

void prosAST_dump(prosAST *self, prosIdTable *it);

void *prosAST_alloc(prosAST *self, size_t n);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_CODE_AST_AST_H
