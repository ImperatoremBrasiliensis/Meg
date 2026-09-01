/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/AST/AST.h>

#include <internal/Code/AST/ASTContext.h>
#include <internal/Code/Lexicon/IdTable.h>
#include <internal/Orbita.h>

#include <stdio.h>

prosAST prosAST_new(prosAllocator *allocator) {
   prosAST ret = {
      .tree = prosArena_new(allocator)
   };

   return ret;
}

void prosAST_del(prosAST *self) {
   prosArena_del(&self->tree);
   *self = (prosAST){};
}

static bool astIsBuiltinType(prosASTType ptr) {
   static const prosASTType tvoid = (prosASTType) &PROS_VOID_TYPE;
   static const prosASTType tint = (prosASTType) &PROS_INT_TYPE;
   static const prosASTType tbool = (prosASTType) &PROS_BOOL_TYPE;
   static const prosASTType tfloat = (prosASTType) &PROS_FLOAT_TYPE;
   static const prosASTType tchar = (prosASTType) &PROS_CHAR_TYPE;

   return ptr == tvoid ||
      ptr == tint ||
      ptr == tbool ||
      ptr == tfloat ||
      ptr == tchar;
}

struct prosASTTypeDesc prosAST_getTypeDecl(prosASTType ptr) {
   constexpr prosASTType flags =
      PROS_CONST_QUALIFIER_FLAG |
      PROS_SIGNED_TYPES_FLAG;

   prosASTType pnoflags = ptr & ~flags;

   return (struct prosASTTypeDesc){
      .as.builtinp = (void *) pnoflags,
      .isBuiltin = astIsBuiltinType(pnoflags),
      .isConst = (ptr & PROS_CONST_QUALIFIER_FLAG) == PROS_CONST_QUALIFIER_FLAG,
      .isSigned = (ptr & PROS_SIGNED_TYPES_FLAG) == PROS_SIGNED_TYPES_FLAG
   };
}

const prosASTBuiltinType PROS_VOID_TYPE = {"", 0, false};
const prosASTBuiltinType PROS_INT_TYPE = {"int", 4, true};
const prosASTBuiltinType PROS_BOOL_TYPE = {"bool", 1, false};
const prosASTBuiltinType PROS_FLOAT_TYPE = {"float", 4, true};
const prosASTBuiltinType PROS_CHAR_TYPE = {"char", 1, false};

prosASTNode *prosAST_addNode(prosAST *self, prosASTNode node) {
   prosASTNode *ptr = prosArena_alloc(&self->tree, sizeof(node));
   *ptr = node;

   if (!self->nodeCount && !self->firstNode)
      self->firstNode = ptr;

   self->nodeCount++;
   return ptr;
}

prosASTNode *prosAST_addCiclicNode(prosAST *self, prosASTCiclicNode **last, prosASTNode node) {
   prosASTCiclicNode *ptr = prosArena_alloc(
      &self->tree,
      sizeof(prosASTCiclicNode)
   );

   if (*last) {
      ptr->next = (*last)->next;
      (*last)->next = ptr;
   } else {
      ptr->next = ptr;
   }

   *last = ptr;
   ptr->node = node;
   return &ptr->node;
}

void *prosAST_alloc(prosAST *self, size_t n) {
   return prosArena_alloc(&self->tree, n);
}

struct astDump {
   prosIdTable *it;
   bool *isLast;
   int depth;
};

static bool astPrintNode(struct astDump *dump, prosASTNode *node);

static void astPrintRoots(struct astDump *dump) {
   int ind = 0;
   while (ind < dump->depth - 1) {
      if (dump->isLast[ind])
         fputs("   ", stdout);
      else
         fputs("\033[37m \u2502 \033[0m", stdout);

      ind++;
   }

   if (dump->isLast[ind])
      fputs("\033[37m \u2570\u2574\033[0m", stdout);
   else
      fputs("\033[37m \u251c\u2574\033[0m", stdout);
}

static void astPrintStatement(prosString name) {
   printf(
      "\033[38;2;190;255;230m"  // Soft green.
      "%s"
      "\033[0m",
      name
   );
}

static void astPrintDecl(prosString name) {
   printf(
      "\033[38;2;255;190;230m"
      "%s"
      "\033[0m",
      name
   );
}

static void astPrintExpr(prosString name) {
   printf(
      "\033[38;2;190;230;255m"
      "%s"
      "\033[0m",
      name
   );
}

static void astPrintAuxiliaryNode(prosString name) {
   printf(
      "\033[38;2;255;255;190m"
      "%s"
      "\033[0m",
      name
   );
}

static prosString astGetTypename(prosASTType type) {
   auto typeDecl = prosAST_getTypeDecl(type);
   prosString typename;
   if (typeDecl.isBuiltin)
      typename = typeDecl.as.builtinp->name;
   else
      pros_panic("prosAST: Support for user-defined types is broken.");

   return typename;
}

static bool astPrintModule(struct astDump *dump, prosASTNode *node) {
   astPrintAuxiliaryNode("Module");
   printf("('%s')\n", node->nodeAs.module.name);

   prosASTCiclicNode *child1 = node->nodeAs.module.decls->next;
   prosASTCiclicNode *child = child1;

   if (child) {
      do {
         dump->isLast[dump->depth] = child->next == child1;
         astPrintNode(dump, &child->node);

         child = child->next;
      } while (child != child1);
   }

   return true;
}

static bool astPrintParamDecl([[maybe_unused]] struct astDump *dump, prosASTNode *node) {
   astPrintDecl("ParamDecl");
   printf(
      " %s : %s\n",
      node->nodeAs.funcParamDecl.identifier->str,
      astGetTypename(node->nodeAs.funcParamDecl.type)
   );
   return true;
}

static bool astPrintFuncDecl(struct astDump *dump, prosASTNode *node) {
   astPrintDecl("FuncDecl");
   printf(
      " %s : (",
      node->nodeAs.funcDecl.identifier->str
   );

   prosASTNode *paramHandle = node->nodeAs.funcDecl.params;

   if (paramHandle && paramHandle->nodeAs.nodeHandle.nodes) {
      prosASTCiclicNode *child1 = paramHandle->nodeAs.nodeHandle.nodes;
      prosASTCiclicNode *child = child1;

      do {
         printf(
            "%s",
            astGetTypename(child->node.nodeAs.funcParamDecl.type)
         );

         if (child->next != child1)
            printf(", ");

         child = child->next;
      } while (child != child1);

      printf(") %s\n", astGetTypename(node->nodeAs.funcDecl.retType));

      dump->isLast[dump->depth] = false;
      astPrintNode(dump, paramHandle);
   } else
      printf(") %s\n", astGetTypename(node->nodeAs.funcDecl.retType));

   dump->isLast[dump->depth] = true;
   if (node->nodeAs.funcDecl.body)
      astPrintNode(dump, node->nodeAs.funcDecl.body);

   return true;
}

static bool astPrintOperation(struct astDump *dump, prosASTNode *node) {
   astPrintAuxiliaryNode("Operation\n");

   prosASTCiclicNode *child1 = node->nodeAs.operation.body->next;
   prosASTCiclicNode *child = child1;

   if (child) {
      do {
         dump->isLast[dump->depth] = child->next == child1;
         astPrintNode(dump, &child->node);

         child = child->next;
      } while (child != child1);
   }

   return true;
}

static bool astPrintValDecl(struct astDump *dump, prosASTNode *node) {
   astPrintDecl("ValDecl");
   printf(
      " %s : %s\n",
      node->nodeAs.valDecl.identifier->str,
      astGetTypename(node->nodeAs.valDecl.type)
   );

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.valDecl.value);
   return true;
}

static bool astPrintVarDecl(struct astDump *dump, prosASTNode *node) {
   astPrintDecl("VarDecl");
   printf(
      " %s : %s\n",
      node->nodeAs.varDecl.identifier->str,
      astGetTypename(node->nodeAs.varDecl.type)
   );

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.varDecl.initializer);
   return true;
}

static bool astPrintDeclRefExpr([[maybe_unused]] struct astDump *dump, prosASTNode *node) {
   prosString objName = nullptr;
   prosASTType type = 0;

   astPrintExpr("DeclRefExpr");

   switch (node->nodeAs.declRefExpr.declRefType) {
   case PROS_AST_DECLREF_FUNC_PARAM:
      type = node->nodeAs.declRefExpr.decl->nodeAs.funcParamDecl.type;
      objName =
         node
            ->nodeAs.declRefExpr.decl
            ->nodeAs.funcParamDecl.identifier->str;

      printf("(ParamDecl) %s : %s\n", objName, astGetTypename(type));
      break;

   case PROS_AST_DECLREF_VAL:
      type = node->nodeAs.declRefExpr.decl->nodeAs.valDecl.type;
      objName =
         node
            ->nodeAs.declRefExpr.decl
            ->nodeAs.valDecl.identifier->str;

      printf("(ValDecl) %s : %s\n", objName, astGetTypename(type));
      break;

   case PROS_AST_DECLREF_VAR:
      type = node->nodeAs.declRefExpr.decl->nodeAs.varDecl.type;
      objName =
         node
            ->nodeAs.declRefExpr.decl
            ->nodeAs.varDecl.identifier->str;

      printf("(VarDecl) %s : %s\n", objName, astGetTypename(type));
      break;

   case PROS_AST_DECLREF_FUNC:
      type = node->nodeAs.declRefExpr.decl->nodeAs.valDecl.type;
      objName =
         node
            ->nodeAs.declRefExpr.decl
            ->nodeAs.funcDecl.identifier->str;
      printf("(FuncDecl) %s : (", objName);

      prosASTNode *paramHandle = node
                                    ->nodeAs.declRefExpr.decl
                                    ->nodeAs.funcDecl.params;
      if (paramHandle && paramHandle->nodeAs.nodeHandle.nodes) {
         prosASTCiclicNode *child1 = paramHandle->nodeAs.nodeHandle.nodes;
         prosASTCiclicNode *child = child1;

         do {
            printf(
               "%s",
               astGetTypename(child->node.nodeAs.funcParamDecl.type)
            );

            if (child->next != child1)
               printf(", ");

            child = child->next;
         } while (child != child1);
      }

      printf(") %s\n", astGetTypename(type));
      break;

   case PROS_AST_DECLREF_NONE:
   default:
      pros_panic("prosAST: `prosAST_dump()` received an invalid DeclRef.");
   }

   return true;
}

static bool astPrintLiteralExpr([[maybe_unused]] struct astDump *dump, prosASTNode *node) {
   astPrintExpr("LiteralExpr");
   printf(
      " %d : %s, usigned: %s\n",
      node->nodeAs.literalExpr.intialValue,
      astGetTypename(node->nodeAs.literalExpr.type),
      node->nodeAs.literalExpr.isSigned ? "false" : "true"
   );
   return true;
}

static bool astPrintParensExpr(struct astDump *dump, prosASTNode *node) {
   astPrintExpr("ParensExpr");
   printf(
      " : %s\n",
      astGetTypename(node->nodeAs.parensExpr.type)
   );

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.parensExpr.childExpr);
   return true;
}

static bool astPrintBinOpExpr(struct astDump *dump, prosASTNode *node) {
   prosString opSym = nullptr;
   switch (node->nodeAs.binOpExpr.binOpType) {
   case PROS_AST_BIN_OP_EQUAL:
      opSym = "=";
      break;
   case PROS_AST_BIN_OP_PLUS:
      opSym = "+";
      break;
   case PROS_AST_BIN_OP_MINUS:
      opSym = "-";
      break;
   case PROS_AST_BIN_OP_STAR:
      opSym = "*";
      break;
   case PROS_AST_BIN_OP_SLASH:
      opSym = "/";
      break;
   case PROS_AST_BIN_OP_PLUSEQUAL:
      opSym = "+=";
      break;
   case PROS_AST_BIN_OP_MINUSEQUAL:
      opSym = "-=";
      break;
   case PROS_AST_BIN_OP_STAREQUAL:
      opSym = "*=";
      break;
   case PROS_AST_BIN_OP_SLASHEQUAL:
      opSym = "/=";
      break;

   default:
      pros_panic("prosAST: `prosAST_dump()` received an invalid node.");
   }

   astPrintExpr("BinOpExpr");
   printf("(%s)\n", opSym);

   dump->isLast[dump->depth] = false;
   astPrintNode(dump, node->nodeAs.binOpExpr.operand1);

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.binOpExpr.operand2);
   return true;
}

static bool astPrintUnaOpExpr(struct astDump *dump, prosASTNode *node) {
   prosString opSym = nullptr;
   switch (node->nodeAs.unaOpExpr.unaOpType) {
   case PROS_AST_UNA_OP_PLUS:
      opSym = "+";
      break;
   case PROS_AST_UNA_OP_MINUS:
      opSym = "-";
      break;
   case PROS_AST_UNA_OP_EXCLAMATION:
      opSym = "!";
      break;
   default:
      pros_panic("prosAST: `prosAST_dump()` received an invalid node.");
   }

   astPrintExpr("UnaOpExpr");
   printf("(%s)\n", opSym);

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.unaOpExpr.operand);
   return true;
}

static bool astPrintCallArgumentExpression(
   struct astDump *dump,
   prosASTNode *node
) {
   astPrintAuxiliaryNode("CallArgumentExpression");
   printf(
      " : %s\n",
      astGetTypename(node->nodeAs.callArgumentExpression.type)
   );

   prosASTNode *child = node->nodeAs.expressionStatement.child;
   if (!child)
      pros_panic("prosAST: `prosAST_dump()` received a nullptr expression child.");

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.callArgumentExpression.child);
   return true;
}

static bool astPrintExpressionStatement(
   struct astDump *dump,
   prosASTNode *node
) {
   astPrintStatement("ExpressionStatement");
   printf(
      " : %s\n",
      astGetTypename(node->nodeAs.expressionStatement.type)
   );

   prosASTNode *child = node->nodeAs.expressionStatement.child;
   if (!child)
      pros_panic("prosAST: `prosAST_dump()` received a nullptr expression child.");

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.expressionStatement.child);
   return true;
}

static bool astPrintCallOpExpr(struct astDump *dump, prosASTNode *node) {
   astPrintExpr("CallOpExpr");
   printf(
      " : %s\n",
      astGetTypename(node->nodeAs.callOpExpr.type)
   );

   dump->isLast[dump->depth] = node->nodeAs.callOpExpr.argCount == 0;
   astPrintNode(dump, node->nodeAs.callOpExpr.funcRef);

   size_t argn = 0;
   while (argn < node->nodeAs.callOpExpr.argCount) {
      if (argn == node->nodeAs.callOpExpr.argCount - 1)
         dump->isLast[dump->depth] = true;
      else
         dump->isLast[dump->depth] = false;

      astPrintNode(
         dump,
         node->nodeAs.callOpExpr.arguments[argn]
      );

      argn++;
   }

   return true;
}

static bool astPrintExpression(struct astDump *dump, prosASTNode *node) {
   astPrintAuxiliaryNode("Expression");
   printf(
      " : %s\n",
      astGetTypename(node->nodeAs.expression.type)
   );

   prosASTNode *child = node->nodeAs.expression.child;
   if (!child)
      pros_panic("prosAST: `prosAST_dump()` received a nullptr expression child.");

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.expression.child);

   return true;
}

static bool astPrintIfStatement(struct astDump *dump, prosASTNode *node) {
   astPrintStatement("IfStatement");

   switch (node->nodeAs.ifStatement.ifType) {
   case PROS_AST_IF_STATEMENT_IF:
      printf("(if)\n");
      dump->isLast[dump->depth] = false;
      astPrintNode(dump, node->nodeAs.ifStatement.expression);

      // Then block.
      dump->isLast[dump->depth] = true;
      astPrintNode(dump, node->nodeAs.ifStatement.then);
      break;
   case PROS_AST_IF_STATEMENT_IF_OR:
      printf("(if or)\n");
      dump->isLast[dump->depth] = false;
      astPrintNode(dump, node->nodeAs.ifStatement.expression);

      // Then block.
      astPrintNode(dump, node->nodeAs.ifStatement.then);

      // Or statement.
      dump->isLast[dump->depth] = true;
      astPrintNode(dump, node->nodeAs.ifStatement.elseBlock);
      break;
   case PROS_AST_IF_STATEMENT_IF_ELSE:
      printf("(if else)\n");
      dump->isLast[dump->depth] = false;
      astPrintNode(dump, node->nodeAs.ifStatement.expression);

      // Then block.
      astPrintNode(dump, node->nodeAs.ifStatement.then);

      // Else block.
      dump->isLast[dump->depth] = true;
      astPrintNode(dump, node->nodeAs.ifStatement.elseBlock);
      break;
   default:
      pros_panic(
         "prosAST: `prosAST_dump()` received an invalid IfStatement node type. "
      );
   }

   return true;
}

static bool astPrintWhileStatement(struct astDump *dump, prosASTNode *node) {
   astPrintStatement("WhileStatement\n");

   dump->isLast[dump->depth] = false;
   astPrintNode(dump, node->nodeAs.whileStatement.expression);

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.whileStatement.doBlock);

   return true;
}

static bool astPrintLoopStatement(struct astDump *dump, prosASTNode *node) {
   astPrintStatement("LoopStatement\n");

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.loopStatement.block);

   return true;
}

static bool astPrintBreakStatement(
   [[maybe_unused]] struct astDump *dump,
   [[maybe_unused]] prosASTNode *node
) {
   astPrintStatement("BreakStatement\n");
   return true;
}

static bool astPrintContinueStatement(
   [[maybe_unused]] struct astDump *dump,
   [[maybe_unused]] prosASTNode *node
) {
   astPrintStatement("ContinueStatement\n");
   return true;
}

static bool astPrintReturnStatement(struct astDump *dump, prosASTNode *node) {
   astPrintStatement("ReturnStatement");
   printf(
      " : %s\n",
      astGetTypename(node
            ->nodeAs.returnStatement.expression
            ->nodeAs.expression.type)
   );

   dump->isLast[dump->depth] = true;
   astPrintNode(dump, node->nodeAs.returnStatement.expression);
   return true;
}

static bool astPrintNodeHandle(struct astDump *dump, prosASTNode *node) {
   astPrintAuxiliaryNode("NodeHandle\n");

   prosASTCiclicNode *child1 = node->nodeAs.nodeHandle.nodes;
   prosASTCiclicNode *child = child1;

   if (child) {
      do {
         dump->isLast[dump->depth] = child->next == child1;
         astPrintNode(dump, &child->node);

         child = child->next;
      } while (child != child1);
   }

   return true;
}

static bool astPrintNode(struct astDump *dump, prosASTNode *node) {
   dump->depth++;
   astPrintRoots(dump);
   switch (node->type) {
   case PROS_AST_NODE_HANDLE:
      astPrintNodeHandle(dump, node);
      break;
   case PROS_AST_SCOPE_MODULE:
      astPrintModule(dump, node);
      break;
   case PROS_AST_SCOPE_OPERATION:
      astPrintOperation(dump, node);
      break;
   case PROS_AST_IF_STATEMENT:
      astPrintIfStatement(dump, node);
      break;
   case PROS_AST_WHILE_STATEMENT:
      astPrintWhileStatement(dump, node);
      break;
   case PROS_AST_LOOP_STATEMENT:
      astPrintLoopStatement(dump, node);
      break;
   case PROS_AST_BREAK_STATEMENT:
      astPrintBreakStatement(dump, node);
      break;
   case PROS_AST_CONTINUE_STATEMENT:
      astPrintContinueStatement(dump, node);
      break;
   case PROS_AST_RETURN_STATEMENT:
      astPrintReturnStatement(dump, node);
      break;
   case PROS_AST_DECL_FUNC:
      astPrintFuncDecl(dump, node);
      break;
   case PROS_AST_DECL_PARAMETER:
      astPrintParamDecl(dump, node);
      break;
   case PROS_AST_DECL_VAL:
      astPrintValDecl(dump, node);
      break;
   case PROS_AST_DECL_VAR:
      astPrintVarDecl(dump, node);
      break;
   case PROS_AST_EXPR_DECL_REF:
      astPrintDeclRefExpr(dump, node);
      break;
   case PROS_AST_EXPR_CALL_OP:
      astPrintCallOpExpr(dump, node);
      break;
   case PROS_AST_CALL_ARGUMENT_EXPRESSION:
      astPrintCallArgumentExpression(dump, node);
      break;
   case PROS_AST_EXPRESSION_STATEMENT:
      astPrintExpressionStatement(dump, node);
      break;
   case PROS_AST_EXPR_BINARY_OPERATOR:
      astPrintBinOpExpr(dump, node);
      break;
   case PROS_AST_EXPR_UNARY_OPERATOR:
      astPrintUnaOpExpr(dump, node);
      break;
   case PROS_AST_EXPR_LITERAL:
      astPrintLiteralExpr(dump, node);
      break;
   case PROS_AST_EXPR_PARENS:
      astPrintParensExpr(dump, node);
      break;
   case PROS_AST_EXPRESSION:
      astPrintExpression(dump, node);
      break;
   default:
      pros_panic("prosAST: `prosAST_dump()` Received an invalid node.");
   }
   dump->depth--;
   return true;
}

void prosAST_dump(prosAST *self, prosIdTable *it) {
   if (self->nodeCount == 0 || !self->firstNode) {
      puts("AST Dump: nothing to print.");
      return;
   }

   puts("\033[1m\nAST Dump:\033[0m");

   static bool isLast[256] = {};
   isLast[0] = true;
   astPrintNode(
      &(struct astDump){
         it,
         isLast,
         0
      },
      self->firstNode
   );
}
