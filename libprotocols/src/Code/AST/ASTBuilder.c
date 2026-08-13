/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/AST/ASTBuilder.h>

#include <internal/Orbita.h>

#include <assert.h>

prosASTBuilder prosASTBuilder_new(prosAllocator *allocator) {
   prosASTBuilder ret;
   ret.ast = prosAST_new(allocator);
   ret.ctx = prosASTContext_new(&ret.ast);
   return ret;
}

void prosASTBuilder_del(prosASTBuilder *self) {
   prosASTContext_del(&self->ctx);
   prosAST_del(&self->ast);
   *self = (prosASTBuilder){};
}

prosASTNode *prosASTBuilder_addNodeHandleNode(prosASTBuilder *self) {
   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_NODE_HANDLE
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addModuleNode(
   prosASTBuilder *self,
   prosString name
) {
   if (self->ast.nodeCount != 0) {
      pros_panic(
         "prosASTBuilder_addModuleNode():"
         " Current AST is not empty and should not have two"
         " modules in the same tree."
      );
   }

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_SCOPE_MODULE,
         .nodeAs.module = {
            .name = name,
            .declCount = 0
         }
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addFuncDecl(
   prosASTBuilder *self,
   prosId *name,
   prosASTType type
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_MODULE) {
      pros_panic(
         "prosASTBuilder_addFuncDecl():"
         " Current AST node is not the module node."
      );
   }

   auto ret = prosAST_addCiclicNode(
      &self->ast,
      &current->nodeAs.module.decls,
      (prosASTNode){
         .type = PROS_AST_DECL_FUNC,
         .nodeAs.funcDecl = {
            .retType = type,
            .identifier = name
         }
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

static prosASTNode **astBuilderNodeNeedsOperation(prosASTNode *node) {
   switch (node->type) {
   case PROS_AST_DECL_FUNC:
      if (node->nodeAs.funcDecl.body)
         break;
      return &node->nodeAs.funcDecl.body;

   case PROS_AST_IF_STATEMENT:
      switch (node->nodeAs.ifStatement.ifType) {
      case PROS_AST_IF_STATEMENT_IF:
         return &node->nodeAs.ifStatement.then;

      case PROS_AST_IF_STATEMENT_IF_OR:
      case PROS_AST_IF_STATEMENT_IF_ELSE:
         if (!node->nodeAs.ifStatement.then)
            return &node->nodeAs.ifStatement.then;
         else if (!node->nodeAs.ifStatement.elseBlock)
            return &node->nodeAs.ifStatement.elseBlock;

      default:
      }
      break;

   case PROS_AST_WHILE_STATEMENT:
      if (node->nodeAs.whileStatement.doBlock)
         break;
      return &node->nodeAs.whileStatement.doBlock;

   case PROS_AST_LOOP_STATEMENT:
      if (node->nodeAs.loopStatement.block)
         break;
      return &node->nodeAs.loopStatement.block;

   default:
   }

   pros_panic(
      "proaASTBuilder:"
      " Current node does not need an operation block."
   );
}

prosASTNode *prosASTBuilder_addOperationNode(prosASTBuilder *self) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   auto nodeToOperationPtrPtr = astBuilderNodeNeedsOperation(current);

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_SCOPE_OPERATION
      }
   );
   *nodeToOperationPtrPtr = ret;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

static prosASTNode *astBuilderAddNodeInOperation(
   prosAST *ast,
   prosASTNode *operation,
   prosASTNode *node
) {
   assert(operation->type == PROS_AST_SCOPE_OPERATION);

   operation->nodeAs.operation.instrCount++;
   return prosAST_addCiclicNode(
      ast,
      &operation->nodeAs.operation.body,
      *node
   );
}

prosASTNode *prosASTBuilder_addFuncParameterDecl(
   prosASTBuilder *self,
   prosId *name,
   prosASTType type,
   size_t paramIndex
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_NODE_HANDLE) {
      pros_panic(
         "prosASTBuilder_addFuncParameterDecl():"
         " Current AST node is not a node handle node."
      );
   }

   if (paramIndex > current->nodeAs.nodeHandle.size) {
      pros_panic(
         "prosASTBuilder_addFuncParameterDecl():"
         " Function alreay have its parameters defined."
         " New parameters to this function are no longer"
         " accepted."
      );
   }

   auto ret = prosAST_addCiclicNode(
      &self->ast,
      &current->nodeAs.nodeHandle.nodes,
      (prosASTNode){
         .type = PROS_AST_DECL_PARAMETER,
         .nodeAs.funcParamDecl = {
            .type = type,
            .identifier = name
         }
      }
   );

   current->nodeAs.nodeHandle.size++;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addObjectDecl(
   prosASTBuilder *self,
   prosId *name,
   prosASTType type,
   bool value
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_OPERATION) {
      pros_panic(
         "prosASTBuilder_addObjectDecl():"
         " Current node is not an operation node."
      );
   }

   prosASTNode node;
   if (value) {
      node = (prosASTNode){
         .type = PROS_AST_DECL_VAL,
         .nodeAs.valDecl = {
            .type = type,
            .identifier = name
         }
      };
   } else {
      node = (prosASTNode){
         .type = PROS_AST_DECL_VAR,
         .nodeAs.varDecl = {
            .type = type,
            .identifier = name
         }
      };
   }

   auto ret = astBuilderAddNodeInOperation(
      &self->ast,
      current,
      &node
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addIfStatementNode(prosASTBuilder *self) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_OPERATION) {
      pros_panic(
         "prosASTBuilder_addIfStatementNode():"
         " Current node is not an operation node."
      );
   }

   auto ret = astBuilderAddNodeInOperation(
      &self->ast,
      current,
      &(prosASTNode){
         .type = PROS_AST_IF_STATEMENT,
         .nodeAs.ifStatement = {
            .ifType = PROS_AST_IF_STATEMENT_IF
         }
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addWhileStatementNode(prosASTBuilder *self) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_OPERATION) {
      pros_panic(
         "prosASTBuilder_addWhileStatementNode():"
         " Current node is not an operation node."
      );
   }

   auto ret = astBuilderAddNodeInOperation(
      &self->ast,
      current,
      &(prosASTNode){
         .type = PROS_AST_WHILE_STATEMENT,
         .nodeAs.whileStatement = {}
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addLoopStatementNode(prosASTBuilder *self) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_OPERATION) {
      pros_panic(
         "prosASTBuilder_addLoopStatementNode():"
         " Current node is not an operation node."
      );
   }

   auto ret = astBuilderAddNodeInOperation(
      &self->ast,
      current,
      &(prosASTNode){
         .type = PROS_AST_LOOP_STATEMENT
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addExpressionStatementNode(prosASTBuilder *self) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_OPERATION) {
      pros_panic(
         "prosASTBuilder_addExpressionStatementNode():"
         " Current node is not an operation node."
      );
   }

   auto ret = astBuilderAddNodeInOperation(
      &self->ast,
      current,
      &(prosASTNode){
         .type = PROS_AST_EXPRESSION_STATEMENT,
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addBreakStatementNode(
   prosASTBuilder *self,
   prosASTNode *loop
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_OPERATION) {
      pros_panic(
         "prosASTBuilder_addBreakStatementNode():"
         " Current node is not an operation node."
      );
   }

   auto ret = astBuilderAddNodeInOperation(
      &self->ast,
      current,
      &(prosASTNode){
         .type = PROS_AST_BREAK_STATEMENT,
         .nodeAs.breakStatement = {
            .loop = loop
         }
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}
prosASTNode *prosASTBuilder_addContinueStatementNode(
   prosASTBuilder *self,
   prosASTNode *loop
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_OPERATION) {
      pros_panic(
         "prosASTBuilder_addContinueStatementNode():"
         " Current node is not an operation node."
      );
   }

   auto ret = astBuilderAddNodeInOperation(
      &self->ast,
      current,
      &(prosASTNode){
         .type = PROS_AST_CONTINUE_STATEMENT,
         .nodeAs.continueStatement = {
            .loop = loop
         }
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addReturnStatementNode(prosASTBuilder *self) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   if (current->type != PROS_AST_SCOPE_OPERATION) {
      pros_panic(
         "prosASTBuilder_addReturnStatementNode():"
         " Current node is not an operation node."
      );
   }

   auto ret = astBuilderAddNodeInOperation(
      &self->ast,
      current,
      &(prosASTNode){
         .type = PROS_AST_RETURN_STATEMENT
      }
   );
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

static prosASTNode **astBuilderParentNodeNeedsExpr(prosASTNode *parent) {
   switch (parent->type) {
   case PROS_AST_DECL_VAL:
      return &parent->nodeAs.valDecl.value;

   case PROS_AST_DECL_VAR:
      return &parent->nodeAs.varDecl.initializer;

   case PROS_AST_IF_STATEMENT:
      return &parent->nodeAs.ifStatement.expression;

   case PROS_AST_WHILE_STATEMENT:
      return &parent->nodeAs.whileStatement.expression;

   case PROS_AST_RETURN_STATEMENT:
      return &parent->nodeAs.returnStatement.expression;

   default:
      pros_panic("prosASTBuilder: Received an invalid node.");
   }
}

prosASTNode *prosASTBuilder_addExpression(
   prosASTBuilder *self,
   prosASTType type
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   auto parentToChildPtrPtr = astBuilderParentNodeNeedsExpr(current);

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_EXPRESSION,
         .nodeAs.expression = {
            .type = type,
            .child = nullptr
         }
      }
   );
   *parentToChildPtrPtr = ret;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode **astBuilderIsParentAnExprNode(prosASTNode *parent) {
   switch (parent->type) {
   case PROS_AST_EXPRESSION:
      if (!parent->nodeAs.expression.child)
         return &parent->nodeAs.expression.child;
      break;

   case PROS_AST_CALL_ARGUMENT_EXPRESSION:
      if (!parent->nodeAs.callArgumentExpression.child)
         return &parent->nodeAs.callArgumentExpression.child;
      break;

   case PROS_AST_EXPRESSION_STATEMENT:
      if (!parent->nodeAs.expressionStatement.child)
         return &parent->nodeAs.expressionStatement.child;
      break;

   case PROS_AST_EXPR_PARENS:
      if (!parent->nodeAs.parensExpr.childExpr)
         return &parent->nodeAs.parensExpr.childExpr;
      break;

   case PROS_AST_EXPR_BINARY_OPERATOR:
      if (!parent->nodeAs.binOpExpr.operand1) {
         return &parent->nodeAs.binOpExpr.operand1;
      } else if (!parent->nodeAs.binOpExpr.operand2)
         return &parent->nodeAs.binOpExpr.operand2;
      break;

   case PROS_AST_EXPR_UNARY_OPERATOR:
      if (!parent->nodeAs.unaOpExpr.operand)
         return &parent->nodeAs.unaOpExpr.operand;
      break;

   case PROS_AST_EXPR_CALL_OP:
      if (!parent->nodeAs.callOpExpr.funcRef)
         return &parent->nodeAs.callOpExpr.funcRef;
      break;

   default:
   }

   pros_panic(
      "prosASTBuilder: Current node is not a kind of expression node."
   );
}

prosASTNode *prosASTBuilder_addDeclRefExpr(
   prosASTBuilder *self,
   prosASTNode *objDecl,
   enum prosASTDeclRefType refType
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   auto parentToChildPtrPtr = astBuilderIsParentAnExprNode(current);

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_EXPR_DECL_REF,
         .nodeAs.declRefExpr = {
            .decl = objDecl,
            .declRefType = refType,
         }
      }
   );
   *parentToChildPtrPtr = ret;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addParensExprNode(
   prosASTBuilder *self,
   prosASTType type
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   auto parentToChildPtrPtr = astBuilderIsParentAnExprNode(current);

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_EXPR_PARENS,
         .nodeAs.parensExpr = {
            .type = type
         }
      }
   );
   *parentToChildPtrPtr = ret;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addBinaryOperatorExprNode(
   prosASTBuilder *self,
   enum prosASTBinOpType binOpType
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   auto parentToChildPtrPtr = astBuilderIsParentAnExprNode(current);

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_EXPR_BINARY_OPERATOR,
         .nodeAs.binOpExpr = {
            .binOpType = binOpType
         }
      }
   );
   *parentToChildPtrPtr = ret;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addUnaryOperatorExprNode(
   prosASTBuilder *self,
   enum prosASTUnaOpType unaOpType
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   auto parentToChildPtrPtr = astBuilderIsParentAnExprNode(current);

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_EXPR_UNARY_OPERATOR,
         .nodeAs.unaOpExpr = {
            .unaOpType = unaOpType
         }
      }
   );
   *parentToChildPtrPtr = ret;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addLiteralExprNode(
   prosASTBuilder *self,
   prosASTType type,
   uint32_t value,
   bool isSigned
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   auto parentToChildPtrPtr = astBuilderIsParentAnExprNode(current);

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_EXPR_LITERAL,
         .nodeAs.literalExpr = {
            .type = type,
            .intialValue = value,
            .isSigned = isSigned
         }
      }
   );

   *parentToChildPtrPtr = ret;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addCallOperatorExprNode(
   prosASTBuilder *self
) {
   prosASTNode *current = prosASTContext_getCurrentNode(&self->ctx);
   auto parentToChildPtrPtr = astBuilderIsParentAnExprNode(current);

   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_EXPR_CALL_OP
      }
   );

   *parentToChildPtrPtr = ret;
   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}

prosASTNode *prosASTBuilder_addCallArgumentExpression(
   prosASTBuilder *self,
   prosASTType type
) {
   auto ret = prosAST_addNode(
      &self->ast,
      (prosASTNode){
         .type = PROS_AST_CALL_ARGUMENT_EXPRESSION,
         .nodeAs.callArgumentExpression = {
            .type = type
         }
      }
   );

   prosASTContext_traverseDown(&self->ctx, ret);
   return ret;
}
