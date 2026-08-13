/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Semantic/Semantics.h>

#include <internal/Code/Analysis/CFG.h>
#include <internal/Orbita.h>
#include <internal/Output.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

prosSemantics prosSemantics_new(prosIdTable *idTable) {
   prosSemantics ret = {};
   ret.allocator = prosAllocator_new();
   ret.scopeStack = prosVector_new(sizeof(prosScope), PROS_MAKE_DEL(prosScope_del));
   ret.builder = prosASTBuilder_new(&ret.allocator);
   ret.idTable = idTable;

   return ret;
}

void prosSemantics_del(prosSemantics *self) {
   prosASTBuilder_del(&self->builder);
   prosVector_del(&self->scopeStack);
   prosAllocator_del(&self->allocator);
   *self = (prosSemantics){};
}

bool prosSemantics_initModuleScope(prosSemantics *self, prosString name) {
   auto node = prosASTBuilder_addModuleNode(&self->builder, name);

   auto scope = prosScope_new(&self->builder, node, nullptr, &self->allocator);
   prosVector_pushBack(&self->scopeStack, &scope);

   self->mainNode = node;
   return true;
}

bool semanticsCheckUpAndCloseScope(prosSemantics *self) {
   if (!self->scopeStack.size)
      return false;
   prosScope *scope = prosVector_getLastObj(&self->scopeStack);

   if (!scope->declTableSize)
      goto popScope;

   auto buk = scope->declList;
   while (buk) {
      prosScopeDecl *decl = buk->decl;

      if (!decl->isUsed) {
         switch (decl->declType) {
         case PROS_SCOPE_DECL_PARAMETER:
            prosOutput_reportWarning(
               decl->id.loc,
               "Unused parameter.",
               nullptr
            );
            break;

         case PROS_SCOPE_DECL_VALUE:
            prosOutput_reportWarning(
               decl->id.loc,
               "Unused value.",
               nullptr
            );
            break;

         case PROS_SCOPE_DECL_VARIABLE:
            prosOutput_reportWarning(
               decl->id.loc,
               "Unused variable.",
               nullptr
            );
            break;

         default:
         }
      }

      buk = buk->next;
   }

popScope:
   prosVector_popBack(&self->scopeStack);
   return true;
}

bool prosSemantics_checkScopeBounds(
   prosSemantics *self,
   prosToken indent
) {
   assert(
      indent.tok == PROS_TOK_INDENT &&
      "`indent` parameter isn't a `PROS_TOK_INDENT`."
   );

   size_t scopeCount = prosVector_getSize(&self->scopeStack);
   if (!scopeCount)
      goto notAssociatedScopeError;

   prosScope *scope = prosVector_getLastObj(&self->scopeStack);
   if (scope->indentation == -1u) {
      scope->indentation = indent.data.identation;
      return false;
   } else {
      goto notAssociatedScopeError;
   }

   return true;

notAssociatedScopeError:
   prosOutput_reportError(
      indent.loc,
      "Attempted to create a not associated scope.",
      nullptr
   );
   return false;
}

bool prosSemantics_actOnOperationScope(
   prosSemantics *self,
   prosToken dedent
) {
   bool ret = true;
   size_t scopeCount = self->scopeStack.size;

   if (!scopeCount) {
      pros_panic(
         "prosSemantics_actOnOperationScope():"
         " No scopes to close."
      );
   }

   prosScope *scope = prosVector_getLastObj(&self->scopeStack);
   if (scope->type != PROS_SCOPE_OPERATION) {
      pros_panic(
         "prosSemantics_actOnOperationScope():"
         " Current scope is not an operation."
      );
   }

   if (scope->indentation <= dedent.data.identation) {
      pros_panic(
         "prosSemantics_actOnOperationScope():"
         " Maybe the scope indentation is wrong."
      );
   }

   if (scope->owner->type == PROS_AST_DECL_FUNC) {
      ret = prosCFG_analyzeFunc(scope->owner, &self->allocator, dedent.loc) ?
         ret :
         false;
   }

   ret = semanticsCheckUpAndCloseScope(self) ? ret : false;
   return ret;
}

bool prosSemantics_actOnEndOfFile(
   prosSemantics *self,
   [[maybe_unused]] prosToken eof
) {
   assert(
      eof.tok == PROS_TOK_EOF &&
      "`indent` parameter isn't a `PROS_TOK_EOF`."
   );

   size_t scopeCount = prosVector_getSize(&self->scopeStack);

   if (!scopeCount) {
      pros_panic(
         "prosSemantics_actOnEndOfFile():"
         " There are no scopes to close."
      );
   }

   for (size_t i = 0; i < scopeCount; i++)
      semanticsCheckUpAndCloseScope(self);
   return true;
}

bool semanticsCreateScope(prosSemantics *self, prosASTNode *owner) {
   prosASTNode *node;

   // TODO: Maybe we can create modules scopes here too.
   switch (owner->type) {
   case PROS_AST_DECL_FUNC:
   case PROS_AST_IF_STATEMENT:
   case PROS_AST_FOR_STATEMENT:
   case PROS_AST_WHILE_STATEMENT:
   case PROS_AST_REPEAT_STATEMENT:
   case PROS_AST_LOOP_STATEMENT:
      node = prosASTBuilder_addOperationNode(&self->builder);
      break;

   default:
      pros_panic(
         "prosSemantics:"
         " Attempted to create a scope that is not an Operation."
      );
   }

   auto scope = prosScope_new(&self->builder, node, owner, &self->allocator);
   prosVector_pushBack(&self->scopeStack, &scope);
   return true;
}

static prosScopeDecl *semanticsHasDeclWithId(
   prosSemantics *self,
   prosToken *id,
   bool shallow
) {
   size_t scopec = self->scopeStack.size;
   if (!scopec)
      pros_panic("prosSemantics: Scope stack is empty.");

   prosScope *scope = (void *) self->scopeStack.data;
   while (scopec-- > 0) {
      auto ret = prosScope_findDecl(&scope[scopec], id->data.id->hash);
      if (ret || shallow) {
         return ret;
      }
   }

   return nullptr;
}

prosScopeDeclType prosSemantics_getIdentifierInfo(
   prosSemantics *self,
   prosToken identifier
) {
   prosScopeDecl *decl = semanticsHasDeclWithId(
      self,
      &identifier,
      false
   );

   return decl ? decl->declType : 0;
}

static inline prosASTType semanticsGetDefinedType(
   prosSemantics *self,
   prosToken id
) {
   pros_panic("prosSemantics: Support for user-defined types is broken.");
   prosScopeDecl decl = *semanticsHasDeclWithId(self, &id, false);

   if (!decl.declType) {
      prosOutput_reportError(
         id.loc,
         "Use of undeclared identifier ('%s').",
         nullptr,
         id.data.id->str
      );
      return -1u;
   }

   else if (decl.declType != PROS_SCOPE_DECL_TYPEDEF) {
      prosOutput_reportError(
         id.loc,
         "'%s' is not a type.",
         nullptr,
         self->idTable,
         id.data.id->str
      );
      prosOutput_reportNote(
         decl.id.loc,
         "Declared here:",
         nullptr
      );
      return -1u;
   }

   return (prosASTType) decl.astNode;
}

static inline prosASTType semanticsGetPrimitiveType(prosToken tok) {
   uintptr_t ret = -1u;

   switch (tok.tok) {
   case PROS_TOK_KEYWORD_INT:
      ret = (prosASTType) &PROS_INT_TYPE;
      break;
   case PROS_TOK_KEYWORD_BOOL:
      ret = (prosASTType) &PROS_BOOL_TYPE;
      break;
   case PROS_TOK_KEYWORD_FLOAT:
      ret = (prosASTType) &PROS_FLOAT_TYPE;
      break;
   case PROS_TOK_KEYWORD_CHAR:
      ret = (prosASTType) &PROS_CHAR_TYPE;
      break;
   default:
   }

   return ret;
}

bool prosSemantics_actOnFuncDecl(
   prosSemantics *self,
   prosToken identifier,
   prosToken returnType,
   prosSemanticsFuncParam params[],
   size_t paramCount
) {
   prosScope *currentScope = prosVector_getLastObj(&self->scopeStack);

   // Is function declared in global scope?
   switch (currentScope->type) {
   case PROS_SCOPE_MODULE:
      break;

   case PROS_SCOPE_UNDEFINED:
      pros_panic("prosSemantics_actOnFuncDecl(): Current scope is undefined.");

   default:
      prosOutput_reportError(
         identifier.loc,
         "Functions must be declared in the module scope.",
         nullptr
      );
      return false;
   }

   // First, has another declaration with the same id?
   prosScopeDecl *decl = semanticsHasDeclWithId(self, &identifier, true);
   if (decl) {
      prosOutput_reportError(
         identifier.loc,
         "Redefinition of '%s':",
         decl->declType ?
            "as another kind of identifier." :
            nullptr,
         decl->id.data.id->str
      );
      prosOutput_reportNote(
         decl->id.loc,
         "Previous declararion is here:",
         nullptr
      );
      return false;
   }

   prosASTType type;
   if (!returnType.tok) {
      type = (prosASTType) &PROS_VOID_TYPE;
   } else if (returnType.tok == PROS_TOK_ID) {
      type = semanticsGetDefinedType(self, returnType);
   } else {
      type = semanticsGetPrimitiveType(returnType);

      if (type == -1u) {
         pros_panic(
            "prosSemantics_actOnFuncDecl(): Received an invalid return type."
         );
      }
   }

   // Adds a function declaration node in the AST.
   prosASTNode *funcNode = prosASTBuilder_addFuncDecl(
      &self->builder,
      identifier.data.id,
      type
   );

   prosScope_pushDecl(
      currentScope,
      &(prosScopeDecl){
         .declType = PROS_SCOPE_DECL_FUNCTION,
         .id = identifier,
         .astNode = funcNode,
         .dataAs.func.typeIndex = type
      }
   );

   // Parameters must be in the function operation scope.
   semanticsCreateScope(self, funcNode);

   if (paramCount) {
      prosASTNode *handle = prosASTBuilder_addNodeHandleNode(&self->builder);

      // Are function parameters valid?
      for (size_t i = 0; i < paramCount; i++) {
         // Has another parameter with the same name?
         for (size_t i2 = 0; i2 < i; i2++) {
            if (params[i2].id.data.id == params[i].id.data.id) {
               prosOutput_reportError(
                  params[i].id.loc,
                  "Redefinition of parameter '%s'.",
                  nullptr,
                  params[i].id.data.id->str
               );
               prosOutput_reportNote(
                  params[i2].id.loc,
                  "Previous declaration is here:",
                  nullptr
               );
               goto nextParam;
            }
         }

         // Is parameter type valid?
         if (params[i].type.tok == PROS_TOK_ID) {
            type = semanticsGetDefinedType(self, params[i].type);
         } else {
            type = semanticsGetPrimitiveType(params[i].type);

            if (type == -1u) {
               pros_panic(
                  "prosSemantics_actOnFuncDecl(): Received an invalid return type."
               );
            }
         }

         // Adds a parameter node in the AST.
         prosASTNode *paramNode = prosASTBuilder_addFuncParameterDecl(
            &self->builder,
            params[i].id.data.id,
            type,
            i
         );
         prosASTContext_traverseUp(&self->builder.ctx);

         prosScope_pushDecl(
            prosVector_getLastObj(&self->scopeStack),
            &(prosScopeDecl){
               .id = params[i].id,
               .declType = PROS_SCOPE_DECL_PARAMETER,
               .astNode = paramNode,
               .dataAs.param.typeIndex = type
            }
         );

nextParam:
      }

      funcNode->nodeAs.funcDecl.params = handle;
      prosASTContext_traverseUp(&self->builder.ctx);
   }

   return true;
}

void prosSemanticsExprBonsaiNode_del(prosSemanticsExprBonsaiNode *self) {
   if (self->type == PROS_SEMANTICS_EXPR_BONSAI_NODE_CALL_OP) {
      prosVector_del(&self->as.callOperator.args);
   }
}

static prosASTType semanticsEvaluateNode(
   prosSemantics *self,
   prosVector *bonsai,
   prosSemanticsExprBonsaiNode *node
);

unsigned long semanticsPower(uint8_t base, uint8_t exponent) {
   unsigned long ret = 1;

   while (exponent > 0) {
      if (exponent & 1)
         ret *= base;

      base *= base;
      exponent >>= 1;
   }

   return ret;
}

static prosASTType semanticsEvaluateLiteral(
   prosSemantics *self,
   prosToken literal,
   bool negative
) {
   prosString lit = literal.data.id->str;
   size_t len = strlen(lit);
   prosASTType type = (prosASTType) &PROS_INT_TYPE;

   unsigned long value = 0;
   bool unsign = false;

   /* Has prefix? */
   if (
      len > 1 && lit[0] == '0' && isalpha(lit[1])
   ) {
      switch (lit[1]) {
      case 'x':
         lit += 2, len -= 2;
         goto litIsHexadecimal;
      case 'b':
         lit += 2, len -= 2;
         goto litIsBinary;
      case 'o':
         lit += 2, len -= 2;
         goto litIsOctal;

      default:
         auto loc = literal.loc;
         loc.column += 2;
         loc.len = 1;
         prosOutput_reportError(
            loc,
            "Use of unknown literal prefix.",
            nullptr
         );
         return 0;
      }
   }

   if (!len) {
      prosOutput_reportError(
         literal.loc,
         "Missing value after literal prefix.",
         nullptr
      );
      return 0;
   }

   /* Has sufix? */
   if (isalpha(lit[len - 1])) {
      switch (lit[len - 1]) {
      case 'u':
      case 'U':
         len--;
         unsign = 0;
         break;
      }
   }

   /* litIsDecimal. */
   for (size_t i = 0; i < len; i++) {
      if (value > (uint32_t) -1u) {
         prosOutput_reportError(
            literal.loc,
            "Maximum literal value is '%u'.",
            nullptr,
            -1
         );
         return 0;
      }

      if (!isdigit(lit[i])) {
         prosOutput_reportError(
            literal.loc,
            "Invalid digit '%c'.",
            nullptr,
            lit[i]
         );
         return 0;
      }

      size_t digit = lit[i] - '0';
      if (digit)
         value += semanticsPower(10, len - i - 1) * digit;
   }
   goto finalize;

litIsHexadecimal:
litIsBinary:
litIsOctal:
   prosOutput_reportError(
      literal.loc,
      "Support for this literal format is unavailable.",
      nullptr
   );
   return false;

finalize:
   if (unsign)
      type |= PROS_SIGNED_TYPES_FLAG;

   prosASTBuilder_addLiteralExprNode(
      &self->builder,
      type,
      negative ?
         (uint32_t) -value :
         (uint32_t) value,
      !unsign
   );

   prosASTContext_traverseUp(&self->builder.ctx);
   return type;
}

static prosASTType semanticsEvaluateCallOperator(
   prosSemantics *self,
   prosSemanticsExprBonsaiNode *node,
   prosVector *bonsai
) {
   prosASTType type = 0;

   prosSemanticsExprBonsaiNode *funcRef = prosVector_getAt(
      bonsai,
      node->as.callOperator.funcRefIndex
   );

   prosScopeDecl *decl = semanticsHasDeclWithId(self, &funcRef->tok, false);
   if (!decl) {
      prosOutput_reportError(
         funcRef->tok.loc,
         "Use of undeclared identifier.",
         nullptr
      );
      return 0;
   }

   if (decl->declType != PROS_SCOPE_DECL_FUNCTION) {
      prosOutput_reportError(
         funcRef->tok.loc,
         "Calling a non-function object.",
         nullptr
      );
      return false;
   }

   struct prosASTFuncDecl *funcDecl = &decl->astNode->nodeAs.funcDecl;

   size_t callArgCount = node->as.callOperator.args.size;
   size_t funcParamCount = 0;
   if (funcDecl->params)
      funcParamCount = funcDecl->params->nodeAs.nodeHandle.size;

   if (funcParamCount != callArgCount) {
      prosOutput_reportError(
         node->tok.loc,
         "Expected %zu arguments, have %zu.",
         nullptr,
         funcParamCount,
         callArgCount
      );
      return 0;
   }

   prosASTNode *callNode = prosASTBuilder_addCallOperatorExprNode(
      &self->builder
   );

   type = semanticsEvaluateNode(self, bonsai, funcRef);
   if (!type)
      return false;

   if (callArgCount) {
      prosASTNode **args = prosAST_alloc(
         &self->builder.ast,
         sizeof(prosASTNode *) * callArgCount
      );
      callNode->nodeAs.callOpExpr.arguments = args;
      callNode->nodeAs.callOpExpr.argCount = callArgCount;

      size_t *argIdxs = (void *) node->as.callOperator.args.data;

      prosASTCiclicNode *param = funcDecl->params->nodeAs.nodeHandle.nodes->next;

      for (size_t i = 0; i < callArgCount; i++) {
         prosSemanticsExprBonsaiNode *argNode =
            prosVector_getAt(bonsai, argIdxs[i]);
         prosASTNode *argExprNode = prosASTBuilder_addCallArgumentExpression(
            &self->builder,
            param->node.nodeAs.funcParamDecl.type
         );

         args[i] = argExprNode;

         // Evaluate the expression.
         auto argType = semanticsEvaluateNode(self, bonsai, argNode);
         if (!argType)
            return 0;

         // Does the argument type match with the parameter type?
         if (argType != param->node.nodeAs.funcParamDecl.type) {
            auto argTypeDesc = prosAST_getTypeDecl(argType);
            auto paramTypeDesc = prosAST_getTypeDecl(
               param->node.nodeAs.funcParamDecl.type
            );

            prosOutput_reportError(
               argNode->tok.loc,
               "Passing '%s' to parameter of type '%s'.",
               nullptr,
               argTypeDesc.isBuiltin ?
                  argTypeDesc.as.builtinp->name :
                  "<custom type unavailable>",
               paramTypeDesc.isBuiltin ?
                  paramTypeDesc.as.builtinp->name :
                  "<custom type unavailable>"
            );
            return 0;
         }

         prosASTContext_traverseUp(&self->builder.ctx);
         param = param->next;
      }
   }

   callNode->nodeAs.callOpExpr.type = type;
   prosASTContext_traverseUp(&self->builder.ctx);
   return type;
}

static prosASTType semanticsEvaluateDeclReference(
   prosSemantics *self,
   prosSemanticsExprBonsaiNode *node
) {
   auto decl = semanticsHasDeclWithId(self, &node->tok, false);
   prosASTType type = 0;
   enum prosASTDeclRefType refType = -1;

   if (!decl)
      goto undeclaredId;

   switch (decl->declType) {
   case PROS_SCOPE_DECL_VARIABLE:
      refType = PROS_AST_DECLREF_VAR;
      type = decl->astNode->nodeAs.varDecl.type;
      break;

   case PROS_SCOPE_DECL_VALUE:
      refType = PROS_AST_DECLREF_VAL;
      type = decl->astNode->nodeAs.valDecl.type;
      break;

   case PROS_SCOPE_DECL_PARAMETER:
      refType = PROS_AST_DECLREF_FUNC_PARAM;
      type = decl->astNode->nodeAs.funcParamDecl.type;
      break;

   case PROS_SCOPE_DECL_FUNCTION:
      refType = PROS_AST_DECLREF_FUNC;
      type = decl->astNode->nodeAs.funcDecl.retType;
      break;

   case PROS_SCOPE_DECL_UNDEFINED:
   default:
      goto undeclaredId;
   }

   decl->isUsed = true;

   prosASTBuilder_addDeclRefExpr(&self->builder, decl->astNode, refType);
   prosASTContext_traverseUp(&self->builder.ctx);
   return type;

undeclaredId:
   prosOutput_reportError(
      node->tok.loc,
      "Use of undeclared ientifier '%s'.",
      nullptr,
      node->tok.data.id->str
   );
   return 0;
}

static prosASTType semanticsEvaluateParensEpxr(
   prosSemantics *self,
   prosSemanticsExprBonsaiNode *node,
   prosVector *bonsai
) {
   prosASTNode *parensNode =
      prosASTBuilder_addParensExprNode(&self->builder, 0);
   prosASTType type = 0;

   prosSemanticsExprBonsaiNode *childExpr = prosVector_getAt(
      bonsai,
      node->as.paren
   );

   type = semanticsEvaluateNode(self, bonsai, childExpr);
   if (!type)
      return 0;

   parensNode->nodeAs.parensExpr.type = type;
   prosASTContext_traverseUp(&self->builder.ctx);
   return type;
}

static prosASTType semanticsEvaluateBinaryOperator(
   prosSemantics *self,
   prosSemanticsExprBonsaiNode *node,
   prosVector *bonsai
) {
   prosASTType type1 = 0, type2 = 0;
   enum prosASTBinOpType opType = 0;
   switch (node->tok.tok) {
   case PROS_TOK_OPT_EQUAL:
      opType = PROS_AST_BIN_OP_EQUAL;
      break;

   case PROS_TOK_OPT_PLUS:
      opType = PROS_AST_BIN_OP_PLUS;
      break;

   case PROS_TOK_OPT_MINUS:
      opType = PROS_AST_BIN_OP_MINUS;
      break;

   case PROS_TOK_OPT_STAR:
      opType = PROS_AST_BIN_OP_STAR;
      break;

   case PROS_TOK_OPT_SLASH:
      opType = PROS_AST_BIN_OP_SLASH;
      break;
   case PROS_TOK_OPT_PLUSEQUAL:
      opType = PROS_AST_BIN_OP_PLUSEQUAL;
      break;

   case PROS_TOK_OPT_MINUSEQUAL:
      opType = PROS_AST_BIN_OP_MINUSEQUAL;
      break;

   case PROS_TOK_OPT_STAREQUAL:
      opType = PROS_AST_BIN_OP_STAREQUAL;
      break;

   case PROS_TOK_OPT_SLASHEQUAL:
      opType = PROS_AST_BIN_OP_SLASHEQUAL;
      break;

   default:
      pros_panic("prosSemantics: Received an invalid binary operator.");
   }

   prosASTNode *opNode =
      prosASTBuilder_addBinaryOperatorExprNode(&self->builder, opType);

   prosSemanticsExprBonsaiNode *operand1Node = prosVector_getAt(
      bonsai,
      node->as.binOp.operand1
   );
   type1 = semanticsEvaluateNode(self, bonsai, operand1Node);
   if (!type1)
      return 0;

   prosSemanticsExprBonsaiNode *operand2Node = prosVector_getAt(
      bonsai,
      node->as.binOp.operand2
   );
   type2 = semanticsEvaluateNode(self, bonsai, operand2Node);
   if (!type2)
      return 0;

   if (type1 != type2) {
      prosOutput_reportError(
         node->tok.loc,
         "Operation with operands of different types.",
         nullptr
      );
      return 0;
   }

   opNode->nodeAs.binOpExpr.type = type1;
   prosASTContext_traverseUp(&self->builder.ctx);
   return type1;
}

static prosASTType semanticsEvaluateUnaryOperator(
   prosSemantics *self,
   prosSemanticsExprBonsaiNode *node,
   prosVector *bonsai
) {
   prosASTType type = 0;
   enum prosASTUnaOpType opType = 0;
   switch (node->tok.tok) {
   case PROS_TOK_OPT_PLUS:
      opType = PROS_AST_UNA_OP_PLUS;
      break;

   case PROS_TOK_OPT_MINUS:
      opType = PROS_AST_UNA_OP_MINUS;
      break;

   case PROS_TOK_OPT_EXCLAMATION:
      opType = PROS_AST_UNA_OP_EXCLAMATION;
      break;

   default:
      pros_panic("prosSemantics: Received an invalid binary operator.");
   }

   prosASTNode *opNode =
      prosASTBuilder_addUnaryOperatorExprNode(&self->builder, opType);

   prosSemanticsExprBonsaiNode *operandNode = prosVector_getAt(
      bonsai,
      node->as.unaOp.operand
   );
   type = semanticsEvaluateNode(self, bonsai, operandNode);
   if (!type)
      return 0;

   opNode->nodeAs.unaOpExpr.type = type;
   prosASTContext_traverseUp(&self->builder.ctx);
   return type;
}

static prosASTType semanticsEvaluateBoolLiteral(
   prosSemantics *self,
   prosSemanticsExprBonsaiNode *node
) {
   auto type = (prosASTType) &PROS_BOOL_TYPE;

   prosASTBuilder_addLiteralExprNode(
      &self->builder,
      type,
      node->as.boolLiteral.isTrue,
      false
   );
   prosASTContext_traverseUp(&self->builder.ctx);
   return type;
}

static prosASTType semanticsEvaluateNode(
   prosSemantics *self,
   prosVector *bonsai,
   prosSemanticsExprBonsaiNode *node
) {
   switch (node->type) {
   case PROS_SEMANTICS_EXPR_BONSAI_NODE_BIN_OP:
      return semanticsEvaluateBinaryOperator(self, node, bonsai);

   case PROS_SEMANTICS_EXPR_BONSAI_NODE_UNA_OP:
      return semanticsEvaluateUnaryOperator(self, node, bonsai);

   case PROS_SEMANTICS_EXPR_BONSAI_NODE_PAREN:
      return semanticsEvaluateParensEpxr(self, node, bonsai);

   case PROS_SEMANTICS_EXPR_BONSAI_NODE_NUMBER_LITERAL:
      return semanticsEvaluateLiteral(self, node->tok, false);

   case PROS_SEMANTICS_EXPR_BONSAI_NODE_DECL_REF:
      return semanticsEvaluateDeclReference(self, node);

   case PROS_SEMANTICS_EXPR_BONSAI_NODE_CALL_OP:
      return semanticsEvaluateCallOperator(self, node, bonsai);

   case PROS_SEMANTICS_EXPR_BONSAI_NODE_BOOL_LITERAL:
      return semanticsEvaluateBoolLiteral(self, node);

   case PROS_SEMANTICS_EXPR_BONSAI_NODE_NONE:
   default:
      pros_panic("prosSemantics: Received an invalid expression bonsai.");
   }
}

static prosASTType semanticsActOnExpressionBonsai(
   prosSemantics *self,
   prosVector *bonsai
) {
   prosSemanticsExprBonsaiNode *node = prosVector_getLastObj(bonsai);

   prosASTType type = 0;
   prosASTNode *exprNode = prosASTBuilder_addExpression(&self->builder, 0);

   type = semanticsEvaluateNode(self, bonsai, node);
   if (!type)
      return 0;

   exprNode->nodeAs.expression.type = type;
   prosASTContext_traverseUp(&self->builder.ctx);
   return type;
}

bool prosSemantics_actOnObjectDecl(
   prosSemantics *self,
   prosToken identifier,
   prosToken typeId,
   bool value,
   prosVector *initializerBonsai
) {
   prosScope *currentScope = (prosScope *) prosVector_getLastObj(&self->scopeStack);
   switch (currentScope->type) {
   case PROS_SCOPE_OPERATION:
      break;
   case PROS_SCOPE_MODULE:
      prosOutput_reportError(
         identifier.loc,
         "Variables in the global scope should be declared with "
         "`static` keyword.",
         nullptr
      );
      return false;
   case PROS_SCOPE_UNDEFINED:
      pros_panic("prosSemantics_actOnObjectDecl(): Current scope is undefined.");
   default:
      prosOutput_reportError(
         identifier.loc,
         "Variables should be declared in function local scope.",
         nullptr
      );
      return false;
   }

   prosScopeDecl *decl = semanticsHasDeclWithId(
      self,
      &identifier,
      true
   );

   if (decl) {
      prosOutput_reportError(
         identifier.loc,
         "Redefination of '%s'",
         (decl->declType == value ?
               PROS_SCOPE_DECL_VALUE :
               PROS_SCOPE_DECL_VALUE) ?
            "as a diferent kind of object." :
            nullptr,
         identifier.data.id->str
      );
      prosOutput_reportNote(decl->id.loc, "Declared here:", nullptr);
      return false;
   }

   prosASTType type = 0;
   if (typeId.tok == PROS_TOK_ID) {
      type = semanticsGetDefinedType(self, typeId);
   } else if (typeId.tok) {
      type = semanticsGetPrimitiveType(typeId);

      if (type == -1u) {
         pros_panic(
            "prosSemantics_actOnObjectDecl(): Received an invalid return type."
         );
      }
   }

   prosASTNode *objNode = prosASTBuilder_addObjectDecl(
      &self->builder,
      identifier.data.id,
      type,
      value
   );

   prosASTType exprType = semanticsActOnExpressionBonsai(self, initializerBonsai);
   if (!exprType)
      return false;

   if (!type && exprType) {
      type = exprType;
      if (value) {
         objNode->nodeAs.valDecl.type = type;
      } else {
         objNode->nodeAs.varDecl.type = type;
      }
   } else {
      if (type && type != exprType) {
         prosOutput_reportError(
            identifier.loc,
            "Initializer expression type does not match with"
            " the object type.",
            nullptr
         );
         return false;
      }
   }

   if (value) {
      prosScope_pushDecl(
         currentScope,
         &(prosScopeDecl){
            .declType = PROS_SCOPE_DECL_VALUE,
            .id = identifier,
            .astNode = objNode,
            .dataAs.valDecl.type = type

         }
      );
   } else {
      prosScope_pushDecl(
         currentScope,
         &(prosScopeDecl){
            .declType = PROS_SCOPE_DECL_VARIABLE,
            .id = identifier,
            .astNode = objNode,
            .dataAs.varDecl.type = type
         }
      );
   }

   prosASTContext_traverseUp(&self->builder.ctx);
   return true;
}

bool prosSemantics_actOnIfStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *exprBonsai
) {
   prosScope *currentScope = prosVector_getLastObj(&self->scopeStack);
   if (currentScope->type != PROS_SCOPE_OPERATION) {
      prosOutput_reportError(
         stmt.loc,
         "If statements must be in a operation scope.",
         nullptr
      );
      return false;
   }

   auto node = prosASTBuilder_addIfStatementNode(&self->builder);

   prosASTType exprType = semanticsActOnExpressionBonsai(self, exprBonsai);
   auto exprTypeDecl = prosAST_getTypeDecl(exprType);
   if (!exprTypeDecl.isBuiltin || exprTypeDecl.as.builtinp != &PROS_BOOL_TYPE) {
      prosOutput_reportError(
         stmt.loc,
         "Expression in an if statement must be boolean (`bool`).",
         nullptr
      );
      return false;
   }

   semanticsCreateScope(self, node);
   return true;
}

bool prosSemantics_actOnOrStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *exprBonsai
) {
   prosScope *currentScope = prosVector_getLastObj(&self->scopeStack);
   if (currentScope->type != PROS_SCOPE_OPERATION) {
      prosOutput_reportError(
         stmt.loc,
         "If statements must be in an operation scope.",
         nullptr
      );
      return false;
   }

   if (!currentScope->node->nodeAs.operation.body)
      goto ifStmtNotFoundErr;

   prosASTNode *ifNode = &currentScope->node->nodeAs.operation.body->node;
   if (ifNode->type != PROS_AST_IF_STATEMENT)
      goto ifStmtNotFoundErr;

   // Traverses down all or's of the statement.
   while (ifNode->nodeAs.ifStatement.ifType == PROS_AST_IF_STATEMENT_IF_OR)
      ifNode = ifNode->nodeAs.ifStatement.elseBlock;

   if (ifNode->nodeAs.ifStatement.ifType != PROS_AST_IF_STATEMENT_IF) {
      prosOutput_reportError(
         stmt.loc,
         "If statement is... fat.",
         "`or` keyword should be after an if statement."
      );
      return false;
   }

   prosASTNode *node = prosASTBuilder_addIfStatementNode(&self->builder);

   prosASTType exprType = semanticsActOnExpressionBonsai(self, exprBonsai);
   auto exprTypeDecl = prosAST_getTypeDecl(exprType);
   if (!exprTypeDecl.isBuiltin || exprTypeDecl.as.builtinp != &PROS_BOOL_TYPE) {
      prosOutput_reportError(
         stmt.loc,
         "Expression in an or statement must be boolean (`bool`).",
         nullptr
      );
      return false;
   }

   ifNode->nodeAs.ifStatement.elseBlock = node;
   ifNode->nodeAs.ifStatement.ifType = PROS_AST_IF_STATEMENT_IF_OR;

   semanticsCreateScope(self, node);
   return true;

ifStmtNotFoundErr:
   prosOutput_reportError(
      stmt.loc,
      "`or` statement must be after an if statement.",
      nullptr
   );
   return false;
}

bool prosSemantics_actOnElseStatement(
   prosSemantics *self,
   prosToken stmt
) {
   prosScope *currentScope = prosVector_getLastObj(&self->scopeStack);
   if (currentScope->type != PROS_SCOPE_OPERATION) {
      prosOutput_reportError(
         stmt.loc,
         "If statements must be in an operation scope.",
         nullptr
      );
      return false;
   }

   if (!currentScope->node->nodeAs.operation.body)
      goto ifStmtNotFoundErr;

   prosASTNode *ifNode = &currentScope->node->nodeAs.operation.body->node;
   if (ifNode->type != PROS_AST_IF_STATEMENT)
      goto ifStmtNotFoundErr;

   // Traverses down all or's of the statement.
   while (ifNode->nodeAs.ifStatement.ifType == PROS_AST_IF_STATEMENT_IF_OR)
      ifNode = ifNode->nodeAs.ifStatement.elseBlock;

   if (ifNode->nodeAs.ifStatement.ifType != PROS_AST_IF_STATEMENT_IF) {
      prosOutput_reportError(
         stmt.loc,
         "If statement is... fat.",
         "`else` statement should be after an if statement."
      );
      return false;
   }
   prosASTContext_traverseDown(&self->builder.ctx, ifNode);

   ifNode->nodeAs.ifStatement.ifType = PROS_AST_IF_STATEMENT_IF_ELSE;
   semanticsCreateScope(self, ifNode);

   return true;

ifStmtNotFoundErr:
   prosOutput_reportError(
      stmt.loc,
      "`else` statement must be after an if statement.",
      nullptr
   );
   return false;
}

bool prosSemantics_actOnWhileStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *exprBonsai
) {
   prosScope *currentScope = prosVector_getLastObj(&self->scopeStack);
   if (currentScope->type != PROS_SCOPE_OPERATION) {
      prosOutput_reportError(
         stmt.loc,
         "While loops must be in an operation scope.",
         nullptr
      );
      return false;
   }

   auto node = prosASTBuilder_addWhileStatementNode(&self->builder);

   prosASTType exprType = semanticsActOnExpressionBonsai(self, exprBonsai);
   auto exprTypeDecl = prosAST_getTypeDecl(exprType);
   if (!exprTypeDecl.isBuiltin || exprTypeDecl.as.builtinp != &PROS_BOOL_TYPE) {
      prosOutput_reportError(
         stmt.loc,
         "Expression in an while statement must be boolean (`bool`).",
         nullptr
      );
      return false;
   }

   semanticsCreateScope(self, node);
   return true;
}

bool prosSemantics_actOnLoopStatement(
   prosSemantics *self,
   prosToken stmt
) {
   prosScope *currentScope = prosVector_getLastObj(&self->scopeStack);
   if (currentScope->type != PROS_SCOPE_OPERATION) {
      prosOutput_reportError(
         stmt.loc,
         "Loops must be in an operation scope.",
         nullptr
      );
   }

   auto node = prosASTBuilder_addLoopStatementNode(&self->builder);
   semanticsCreateScope(self, node);
   return true;
}

bool prosSemantics_actOnExpressionStatement(
   prosSemantics *self,
   prosToken stmt [[maybe_unused]],
   prosVector *bonsai
) {
   prosScope *currentScope = prosVector_getLastObj(&self->scopeStack);
   if (currentScope->type != PROS_SCOPE_OPERATION) {
      pros_panic(
         "prosSemantics_actOnExpressionStatement():"
         " It's not expected an expression statement,"
         " except in an operation scope."
      );
   }

   prosASTNode *node = prosASTBuilder_addExpressionStatementNode(&self->builder);
   prosASTType type = semanticsEvaluateNode(self, bonsai, prosVector_getLastObj(bonsai));
   if (!type)
      return false;

   node->nodeAs.expressionStatement.type = type;
   prosASTContext_traverseUp(&self->builder.ctx);
   return true;
}

bool prosSemantics_actOnBreakStatement(
   prosSemantics *self,
   prosToken stmt
) {
   prosASTNode *node = nullptr;

   // Looks up for the loop.
   size_t i = self->scopeStack.size;
   assert(i != 0 && "Scope stack is empty.");

   while (--i > 0) {
      prosScope *scope = prosVector_getAt(&self->scopeStack, i);
      prosASTNode *owner = scope->owner;

      if (
         owner->type == PROS_AST_WHILE_STATEMENT ||
         owner->type == PROS_AST_FOR_STATEMENT ||
         owner->type == PROS_AST_RETURN_STATEMENT ||
         owner->type == PROS_AST_REPEAT_STATEMENT
      ) {
         node = owner;
      }
   }

   if (!node) {
      prosOutput_reportError(
         stmt.loc,
         "Break statement must be in a loop scope.",
         "Put it in a while, for, repeat or simple loop."
      );
      return false;
   }

   prosASTBuilder_addBreakStatementNode(&self->builder, node);
   prosASTContext_traverseUp(&self->builder.ctx);
   return true;
}

bool prosSemantics_actOnContinueStatement(
   prosSemantics *self,
   prosToken stmt
) {
   prosASTNode *node = nullptr;

   // Looks up for the loop.
   size_t i = self->scopeStack.size;
   assert(i != 0 && "Scope stack is empty.");

   while (--i > 0) {
      prosScope *scope = prosVector_getAt(&self->scopeStack, i);
      prosASTNode *owner = scope->owner;

      if (
         owner->type == PROS_AST_WHILE_STATEMENT ||
         owner->type == PROS_AST_FOR_STATEMENT ||
         owner->type == PROS_AST_RETURN_STATEMENT ||
         owner->type == PROS_AST_REPEAT_STATEMENT
      ) {
         node = owner;
      }
   }

   if (!node) {
      prosOutput_reportError(
         stmt.loc,
         "Continue statement must be in a loop scope.",
         "Put it in a while, for, repeat or simple loop."
      );
      return false;
   }

   prosASTBuilder_addContinueStatementNode(&self->builder, node);
   prosASTContext_traverseUp(&self->builder.ctx);
   return true;
}

bool prosSemantics_actOnReturnStatement(
   prosSemantics *self,
   prosToken stmt,
   prosVector *bonsai
) {
   prosScope *currentScope = prosVector_getLastObj(&self->scopeStack);
   if (currentScope->type != PROS_SCOPE_OPERATION) {
      prosOutput_reportError(
         stmt.loc,
         "Return statement is not in a function.",
         nullptr
      );
      return false;
   }

   // Gets current function.
   prosASTNode *funcNode = nullptr;

   size_t i = self->scopeStack.size;
   assert(i != 0 && "Scope stack is empty.");

   while (--i > 0) {
      prosScope *scope = prosVector_getAt(&self->scopeStack, i);
      prosASTNode *owner = scope->owner;
      if (owner->type == PROS_AST_DECL_FUNC) {
         funcNode = owner;
         break;
      }
   }

   if (!funcNode) {
      pros_panic(
         "prosSemantics_actOnReturnStatement():"
         " Return statement is not inside a function,"
         " this must not happen even when the user's"
         " code is wrong."
      );
   }

   prosASTBuilder_addReturnStatementNode(&self->builder);

   prosASTType type = semanticsActOnExpressionBonsai(self, bonsai);
   if (!type)
      return false;

   if (type != funcNode->nodeAs.funcDecl.retType) {
      struct prosASTTypeDesc bonsaiTy = prosAST_getTypeDecl(type);
      struct prosASTTypeDesc funcTy = prosAST_getTypeDecl(
         funcNode->nodeAs.funcDecl.retType
      );

      prosOutput_reportError(
         stmt.loc,
         "Returning '%s' from function of type '%s'.",
         nullptr,
         bonsaiTy.isBuiltin ?
            bonsaiTy.as.builtinp->name :
            "<custom type unavailable>",
         funcTy.isBuiltin ?
            funcTy.as.builtinp->name :
            "<custom type unavailable>"
      );
      return false;
   }

   prosASTContext_traverseUp(&self->builder.ctx);
   return true;
}
