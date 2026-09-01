/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Parse/Parser.h>

#include <internal/Code/Lexicon/Token.h>
#include <internal/Code/Semantic/Scope.h>
#include <internal/Orbita.h>
#include <internal/Output.h>

#include <assert.h>
#include <string.h>

prosParser prosParser_new(
   prosSource *src,
   prosVirtualMachine *vm,
   prosIdTable *idTable
) {
   prosParser ret = {
      .lex = prosLexer_new(vm, src, idTable),
      .sem = prosSemantics_new(idTable),
      .src = src,
      .vm = vm
   };
   return ret;
}

void prosParser_del(prosParser *self) {
   prosLexer_del(&self->lex);
   prosSemantics_del(&self->sem);
   *self = (prosParser){};
}

static inline prosToken parserGetToken(prosLexer *lex) {
   if (lex->tokp >= lex->tokCount)
      return (prosToken){};

   return lex->tokArray[lex->tokp];
}

static inline prosToken parserGetAndPopToken(prosLexer *lex) {
   if (lex->tokp >= lex->tokCount)
      return (prosToken){};

   return lex->tokArray[lex->tokp++];
}

static bool parserExpectationMatch(
   prosParser *self,
   prosVector *stack,
   prosLexer *lex,
   prosToken *token
) {
   if (stack && stack->size) {
      prosTokenTok tok = *(prosTokenTok *) prosVector_getLastObj(stack);
      prosVector_popBack(stack);

      while (true) {
         prosToken t = parserGetToken(lex);
         if (t.tok == tok) {
            *token = parserGetAndPopToken(lex);
            return true;
         }

         if (!t.tok || t.tok == PROS_TOK_EOL) {
            prosLexer_getLine(&self->lex);
            continue;

         } else if (t.tok == PROS_TOK_INDENT || t.tok == PROS_TOK_DEDENT) {
            parserGetAndPopToken(lex);
            continue;
         }

         *token = t;
         return false;
      }
   }

   prosToken t = parserGetToken(lex);
   if (!t.tok) {
      prosLexer_getLine(&self->lex);
      t = parserGetToken(lex);
   }

   if (t.tok == PROS_TOK_EOL || t.tok == PROS_TOK_EOF) {
      *token = parserGetAndPopToken(lex);
      return true;
   }

   *token = t;
   return false;
}

static bool parserIsPrimitiveType(prosTokenTok tok) {
   switch (tok) {
   case PROS_TOK_KEYWORD_INT:
   case PROS_TOK_KEYWORD_BOOL:
   case PROS_TOK_KEYWORD_FLOAT:
   case PROS_TOK_KEYWORD_CHAR:
      return true;

   default:
      return false;
   }
}

static bool parserParseOperationScope(
   prosParser *self,
   prosLexer *lex
);

static bool parserParseFuncDecl(
   prosParser *self,
   prosLexer *lex
) {
   bool ret = true;
   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);
   prosVector params = prosVector_new(sizeof(prosSemanticsFuncParam), nullptr);
   prosToken funcId;

   prosToken tok = {};
   prosVector_pushArray(
      &expectStack,
      &(prosTokenTok[]){
         PROS_TOK_SCOPE_PAREN_CLOSE,
         PROS_TOK_SCOPE_PAREN_OPEN,
         PROS_TOK_ID
      },
      3
   );

   /*
    * Assuming that the caller have already
    * processed the `func` keyword.
    */
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(tok.loc, "Expected function name.", nullptr);
      ret = false;
      goto doReturn;
   }
   funcId = tok;

   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected function parameters:",
         "with '('."
      );
      ret = false;
      goto doReturn;
   }

   /*
    * The compiler should expect the parameter
    * name or the end of the function parameter
    * declarations scope (the ')' character).
    */
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
parseParam:
      prosVector_pushArray(
         &expectStack,
         &(prosTokenTok[]){
            PROS_TOK_PUNCT_COMMA,
            PROS_TOK_ID,
            PROS_TOK_PUNCT_COLON,
            PROS_TOK_ID
         },
         4
      );

      // Parameter name.
      if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
         prosOutput_reportError(
            tok.loc,
            "Expected parameter name.",
            nullptr
         );
         ret = false;
         goto doReturn;
      }
      prosSemanticsFuncParam param = {.id = tok};

      // Expects ':'.
      if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
         prosOutput_reportError(
            tok.loc,
            "Expeted ':' followed by parameter type:",
            "All parameters should declare its type."
         );
         ret = false;
         goto doReturn;
      }

      // Expects type name.
      if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
         // Is primitive type?
         if (!parserIsPrimitiveType(tok.tok)) {
            prosOutput_reportError(
               tok.loc,
               "Expected type identifier of the parameter type.",
               nullptr
            );
            ret = false;
            goto doReturn;
         }

         parserGetAndPopToken(lex);
      }
      param.type = tok;

      // Puts in the vector.
      prosVector_pushBack(&params, &param);

      // Expects comma.
      if (parserExpectationMatch(self, &expectStack, lex, &tok)) {
         goto parseParam;
      }

      tok = parserGetToken(lex);
      if (tok.tok != PROS_TOK_SCOPE_PAREN_CLOSE) {
         prosOutput_reportError(
            tok.loc,
            "Expected ')'.",
            nullptr
         );
         ret = false;
         goto doReturn;
      }
      parserGetAndPopToken(lex);  // Skips the ')'.
   }

   prosToken retType;
   prosVector_pushArray(
      &expectStack,
      &(prosTokenTok[]){
         PROS_TOK_PUNCT_COLON,
         PROS_TOK_ID
      },
      2
   );

   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      if (tok.tok == PROS_TOK_PUNCT_COLON) {
         retType = (prosToken){};
         goto getColon;
      }

      if (!parserIsPrimitiveType(tok.tok)) {
         prosOutput_reportError(
            tok.loc,
            "Expected function return type or ':'.",
            nullptr
         );
      }
      parserGetAndPopToken(lex);
   }
   retType = tok;

getColon:
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected function type or function ':'.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }

   size_t paramsCount = params.size;
   if (
      !prosSemantics_actOnFuncDecl(
         &self->sem,
         funcId,
         retType,
         paramsCount ? (void *) params.data : nullptr,
         params.size
      )
   ) {
      ret = false;
      goto doReturn;
   }

   // Function operation.
   parserParseOperationScope(self, lex);

doReturn:
   prosVector_del(&params);
   prosVector_del(&expectStack);
   return ret;
}

static bool parserParseExpression(
   prosParser *self,
   prosVector *bonsai,
   prosVector *expectStack,
   prosLexer *lex,
   size_t precedence
);

struct parserExpr {
   prosTokenTok tok;
   uint8_t precedence;
   bool (*parse)(
      prosParser *self,
      prosVector *bonsai,
      prosVector *expectStack,
      prosLexer *lex
   );
};

static inline struct parserExpr parserGetNudTokenDef(prosTokenTok opTok);
static inline struct parserExpr parserGetLedTokenDef(prosTokenTok opTok);

static bool parserGetDeclRef(
   prosParser *self,
   prosVector *bonsai,
   prosVector *expectStack,
   prosLexer *lex
) {
   prosToken tok = {};

   prosVector_pushBack(expectStack, &(prosTokenTok){PROS_TOK_ID});
   if (!parserExpectationMatch(self, expectStack, lex, &tok)) {
      pros_panic(
         "prosParser: Expected `PROS_TOK_ID` while "
         "parsing declaration reference."
      );
   }

   // Peek the next token and define reference type.
   prosSemanticsExprBonsaiNodeType nodeType = PROS_SEMANTICS_EXPR_BONSAI_NODE_DECL_REF;

   prosVector_pushBack(
      bonsai,
      &(prosSemanticsExprBonsaiNode){
         .tok = tok,
         .type = nodeType,
      }
   );
   return true;
}

static bool parserGetNumberLiteral(
   prosParser *self,
   prosVector *bonsai,
   prosVector *expectStack,
   prosLexer *lex
) {
   prosToken tok = {};

   prosVector_pushBack(expectStack, &(prosTokenTok){PROS_TOK_LITERAL_NUMBER});
   if (!parserExpectationMatch(self, expectStack, lex, &tok)) {
      pros_panic(
         "prosParser: Expected `PROS_TOK_LITERAL_NUMBER` while "
         "parsing declaration reference."
      );
   }

   /*
    * Number literals are analized by the semantics
    * module. Just pushBack it.
    */
   prosVector_pushBack(
      bonsai,
      &(prosSemanticsExprBonsaiNode){
         .tok = tok,
         .type = PROS_SEMANTICS_EXPR_BONSAI_NODE_NUMBER_LITERAL
      }
   );
   return true;
}

static inline bool parserIsTokenOperator(prosTokenTok tok) {
   switch (tok) {
   case PROS_TOK_OPT_EQUAL:
   case PROS_TOK_OPT_PLUS:
   case PROS_TOK_OPT_MINUS:
   case PROS_TOK_OPT_STAR:
   case PROS_TOK_OPT_SLASH:
   case PROS_TOK_OPT_EXCLAMATION:
   case PROS_TOK_OPT_PLUSEQUAL:
   case PROS_TOK_OPT_MINUSEQUAL:
   case PROS_TOK_OPT_STAREQUAL:
   case PROS_TOK_OPT_SLASHEQUAL:
      return true;
   default:
      return false;
   }
}

static bool parserGetBinaryOperator(
   prosParser *self,
   prosVector *bonsai,
   prosVector *expectStack,
   prosLexer *lex
) {
   prosToken tok = parserGetToken(lex);

   if (!parserIsTokenOperator(tok.tok)) {
      pros_panic(
         "prosParser: Received a token that is not an operator "
         "while parsing binary expression."
      );
   }
   parserGetAndPopToken(lex);

   // Gets the operand1 from the last bonsai's node.
   size_t lhs = bonsai->size;
   if (!lhs--) {
      prosOutput_reportError(
         tok.loc,
         "Expected left hand-side operand.",
         nullptr
      );
      return false;
   }

   if (
      !parserParseExpression(
         self,
         bonsai,
         expectStack,
         lex,
         parserGetLedTokenDef(tok.tok).precedence
      )
   ) {
      prosOutput_reportError(
         parserGetAndPopToken(lex).loc,
         "Expected expression.",
         nullptr
      );
      return false;
   }

   size_t rhs = bonsai->size - 1;

   prosVector_pushBack(
      bonsai,
      &(prosSemanticsExprBonsaiNode){
         .tok = tok,
         .type = PROS_SEMANTICS_EXPR_BONSAI_NODE_BIN_OP,
         .as.binOp = {
            .operand1 = lhs,
            .operand2 = rhs
         }
      }
   );
   return true;
}

static bool parserGetUnaryOperator(
   prosParser *self,
   prosVector *bonsai,
   prosVector *expectStack,
   prosLexer *lex
) {
   prosToken tok = parserGetToken(lex);

   if (!parserIsTokenOperator(tok.tok)) {
      pros_panic(
         "prosParser: Received a token that is not an operator "
         "while parsing unary expression."
      );
   }
   parserGetAndPopToken(lex);

   if (
      !parserParseExpression(
         self,
         bonsai,
         expectStack,
         lex,
         parserGetNudTokenDef(tok.tok).precedence
      )
   ) {
      prosOutput_reportError(
         parserGetToken(lex).loc,
         "Expected expression.",
         nullptr
      );
      return false;
   }

   size_t operand = bonsai->size - 1;

   prosVector_pushBack(
      bonsai,
      &(prosSemanticsExprBonsaiNode){
         .tok = tok,
         .type = PROS_SEMANTICS_EXPR_BONSAI_NODE_UNA_OP,
         .as.unaOp = {
            .operand = operand
         }
      }
   );
   return true;
}

static bool parserGetParenExpr(
   prosParser *self,
   prosVector *bonsai,
   prosVector *expectStack,
   prosLexer *lex
) {
   prosToken tok = parserGetToken(lex);
   assert(
      tok.tok == PROS_TOK_SCOPE_PAREN_OPEN &&
      "Expected '('. Not a paren expression."
   );

   parserGetAndPopToken(lex);
   if (!parserParseExpression(self, bonsai, expectStack, lex, 0)) {
      prosOutput_reportError(
         tok.loc,
         "Expected expression.",
         nullptr
      );
      return false;
   }
   size_t child = bonsai->size - 1;

   tok = parserGetToken(lex);
   if (tok.tok != PROS_TOK_SCOPE_PAREN_CLOSE) {
      prosOutput_reportError(
         tok.loc,
         "Expected ')'.",
         nullptr
      );
      return false;
   }

   prosVector_pushBack(
      bonsai,
      &(prosSemanticsExprBonsaiNode){
         .tok = tok,
         .type = PROS_SEMANTICS_EXPR_BONSAI_NODE_PAREN,
         .as.paren = child
      }
   );
   parserGetAndPopToken(lex);
   return true;
}

static bool parserGetCallOperator(
   prosParser *self,
   prosVector *bonsai,
   prosVector *expectStack,
   prosLexer *lex
) {
   prosToken tok = parserGetToken(lex);
   assert(
      tok.tok == PROS_TOK_SCOPE_PAREN_OPEN &&
      "Expected '('. Not a call expr."
   );

   prosSemanticsExprBonsaiNode *prev = prosVector_getLastObj(bonsai);
   size_t prevIndex = bonsai->size - 1;
   if (
      prev &&
      prev->type == PROS_SEMANTICS_EXPR_BONSAI_NODE_DECL_REF
   ) {
      parserGetAndPopToken(lex);
      tok = parserGetToken(lex);

      prosVector args = prosVector_new(sizeof(size_t), nullptr);
      if (tok.tok != PROS_TOK_SCOPE_PAREN_CLOSE) {
getArg:
         if (parserParseExpression(self, bonsai, expectStack, lex, 0)) {
            prosVector_pushBack(
               &args,
               &(size_t){bonsai->size - 1}
            );
         } else {
            prosOutput_reportError(
               tok.loc,
               "Expected expression.",
               nullptr
            );
            prosVector_del(&args);
            return false;
         }

         prosVector_pushBack(expectStack, &(prosTokenTok){PROS_TOK_PUNCT_COMMA});
         if (parserExpectationMatch(self, expectStack, lex, &tok))
            goto getArg;

         if (tok.tok != PROS_TOK_SCOPE_PAREN_CLOSE) {
            prosOutput_reportError(tok.loc, "Expected ')'.", nullptr);
            prosVector_del(&args);
            return false;
         }
      }

      // Skips the ')'.
      parserGetAndPopToken(lex);

      prosVector_pushBack(
         bonsai,
         &(prosSemanticsExprBonsaiNode){
            .tok = tok,
            .type = PROS_SEMANTICS_EXPR_BONSAI_NODE_CALL_OP,
            .as.callOperator = {
               .args = args,
               .funcRefIndex = prevIndex
            }
         }
      );
      return true;
   }

   prosOutput_reportError(
      tok.loc,
      "Attempt to invoke call operator on non-function "
      "declaration reference.",
      nullptr
   );
   return false;
}

static bool parserGetBooleanLiteral(
   prosParser *self [[maybe_unused]],
   prosVector *bonsai,
   prosVector *expectStack [[maybe_unused]],
   prosLexer *lex
) {
   prosToken tok = parserGetToken(lex);
   assert(
      (
         tok.tok == PROS_TOK_KEYWORD_TRUE ||
         tok.tok == PROS_TOK_KEYWORD_FALSE
      ) &&
      "Not a boolean literal."
   );

   parserGetAndPopToken(lex);

   prosVector_pushBack(
      bonsai,
      &(prosSemanticsExprBonsaiNode){
         .type = PROS_SEMANTICS_EXPR_BONSAI_NODE_BOOL_LITERAL,
         .tok = tok,
         .as.boolLiteral = {
            .isTrue = tok.tok == PROS_TOK_KEYWORD_TRUE
         }
      }
   );
   return true;
}

static const struct parserExpr parserNUD_TOKENS[] = {
   {},

   // Primary exprs.
   {PROS_TOK_ID, 1, parserGetDeclRef},
   {PROS_TOK_LITERAL_NUMBER, 1, parserGetNumberLiteral},
   {PROS_TOK_KEYWORD_TRUE, 1, parserGetBooleanLiteral},
   {PROS_TOK_KEYWORD_FALSE, 1, parserGetBooleanLiteral},

   // Unary operators.
   {PROS_TOK_OPT_PLUS, 4, parserGetUnaryOperator},
   {PROS_TOK_OPT_MINUS, 4, parserGetUnaryOperator},
   {PROS_TOK_OPT_EXCLAMATION, 4, parserGetUnaryOperator},

   // Parens expr.
   {PROS_TOK_SCOPE_PAREN_OPEN, 5, parserGetParenExpr}
};

static const struct parserExpr parserLED_TOKENS[] = {
   {},

   // Binary operators.
   {PROS_TOK_OPT_PLUS, 2, parserGetBinaryOperator},
   {PROS_TOK_OPT_MINUS, 2, parserGetBinaryOperator},
   {PROS_TOK_OPT_STAR, 3, parserGetBinaryOperator},
   {PROS_TOK_OPT_SLASH, 3, parserGetBinaryOperator},

   // Function call operator.
   {PROS_TOK_SCOPE_PAREN_OPEN, 5, parserGetCallOperator},

   // Assignment operators.
   {PROS_TOK_OPT_EQUAL, 6, parserGetBinaryOperator},
   {PROS_TOK_OPT_PLUSEQUAL, 6, parserGetBinaryOperator},
   {PROS_TOK_OPT_MINUSEQUAL, 6, parserGetBinaryOperator},
   {PROS_TOK_OPT_STAREQUAL, 6, parserGetBinaryOperator},
   {PROS_TOK_OPT_SLASHEQUAL, 6, parserGetBinaryOperator}
};

static inline struct parserExpr parserGetNudTokenDef(prosTokenTok opTok) {
   constexpr size_t tableSize =
      sizeof parserNUD_TOKENS / sizeof parserNUD_TOKENS[0];

   for (size_t i = 0; i < tableSize; i++)
      if (parserNUD_TOKENS[i].tok == opTok)
         return parserNUD_TOKENS[i];

   return parserNUD_TOKENS[0];
}

static inline struct parserExpr parserGetLedTokenDef(prosTokenTok opTok) {
   constexpr size_t tableSize =
      sizeof parserLED_TOKENS / sizeof parserLED_TOKENS[0];

   for (size_t i = 0; i < tableSize; i++)
      if (parserLED_TOKENS[i].tok == opTok)
         return parserLED_TOKENS[i];

   return parserLED_TOKENS[0];
}

static bool parserParseExpression(
   prosParser *self,
   prosVector *bonsai,
   prosVector *expectStack,
   prosLexer *lex,
   size_t precedence
) {
   prosToken tok = {};
   size_t tsize = bonsai->size;

   prosVector_pushBack(expectStack, &(prosTokenTok){PROS_TOK_NONE});
   if (parserExpectationMatch(self, expectStack, lex, &tok)) {
      pros_panic("prosParser: Received `PROS_TOK_NONE` while paring expression.");
   }

   auto left = parserGetNudTokenDef(tok.tok);
   if (!left.tok || !left.parse) {
      prosOutput_reportError(
         tok.loc,
         "Expected expression before operator.",
         nullptr
      );
      return false;
   }

   if (!left.parse(self, bonsai, expectStack, lex))
      return false;

   while (true) {
      tok = parserGetToken(lex);
      left = parserGetLedTokenDef(tok.tok);
      if (left.precedence > precedence && left.parse)
         if (left.parse(self, bonsai, expectStack, lex))
            continue;

      break;
   }

   return tsize != bonsai->size;
}

static bool parserParseObjectDecl(
   prosParser *self,
   prosLexer *lex,
   bool value
) {
   bool ret = true;
   prosToken tok, stmt = parserGetAndPopToken(lex);
   prosToken objId = {}, objType = {};
   assert(
      (
         stmt.tok == PROS_TOK_KEYWORD_VAL ||
         stmt.tok == PROS_TOK_KEYWORD_VAR
      ) &&
      "Not a val/var keyword token."
   );

   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);
   prosVector exprBonsai = prosVector_new(
      sizeof(prosSemanticsExprBonsaiNode),
      PROS_MAKE_DEL(prosSemanticsExprBonsaiNode_del)
   );

   prosVector_pushArray(
      &expectStack,
      &(prosTokenTok[]){
         PROS_TOK_PUNCT_COLON,
         PROS_TOK_ID
      },
      2
   );

   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected object name.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }
   objId = tok;

   if (parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosVector_pushBack(&expectStack, &(prosTokenTok){PROS_TOK_ID});
      if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
         if (parserIsPrimitiveType(tok.tok)) {
            parserGetAndPopToken(lex);
         } else {
            prosOutput_reportError(
               tok.loc,
               "Expected object type.",
               nullptr
            );
            ret = false;
            goto doReturn;
         }
      }
      objType = tok;
   }

   prosVector_pushBack(&expectStack, &(prosTokenTok){PROS_TOK_OPT_EQUAL});
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected '='.",
         "Objects must be initialized."
      );
      ret = false;
      goto doReturn;
   }

   exprBonsai = prosVector_new(
      sizeof(prosSemanticsExprBonsaiNode),
      PROS_MAKE_DEL(prosSemanticsExprBonsaiNode_del)
   );
   if (!parserParseExpression(self, &exprBonsai, &expectStack, lex, 1)) {
      ret = false;
      goto doReturn;
   }

   if (exprBonsai.size) {
      prosSemantics_actOnObjectDecl(
         &self->sem,
         objId,
         objType,
         value,
         &exprBonsai
      );
   }

doReturn:
   prosVector_del(&exprBonsai);
   prosVector_del(&expectStack);
   return ret;
}

static bool parserParseIfStatement(prosParser *self, prosLexer *lex) {
   bool ret = true;
   prosToken tok, stmt = parserGetAndPopToken(lex);

   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);
   prosVector bonsai = prosVector_new(
      sizeof(prosSemanticsExprBonsaiNode),
      PROS_MAKE_DEL(prosSemanticsExprBonsaiNode_del)
   );

   if (!parserParseExpression(self, &bonsai, &expectStack, lex, 1)) {
      prosOutput_reportError(
         parserGetToken(lex).loc,
         "Expected expression.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }

   prosVector_pushBack(&expectStack, &(prosTokenTok){PROS_TOK_PUNCT_COLON});
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected ':'.",
         "Any if statement should be followed by a block."
      );
      ret = false;
      goto doReturn;
   }

   if (
      !prosSemantics_actOnIfStatement(&self->sem, stmt, &bonsai) ||
      !parserParseOperationScope(self, lex)
   ) {
      ret = false;
      goto doReturn;
   }

doReturn:
   prosVector_del(&expectStack);
   prosVector_del(&bonsai);
   return ret;
}

static bool parserParseOrStatement(prosParser *self, prosLexer *lex) {
   bool ret = true;
   prosToken tok, stmt = parserGetAndPopToken(lex);
   assert(stmt.tok == PROS_TOK_KEYWORD_OR && "Not a or keyword token.");

   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);
   prosVector bonsai = prosVector_new(
      sizeof(prosSemanticsExprBonsaiNode),
      PROS_MAKE_DEL(prosSemanticsExprBonsaiNode_del)
   );

   if (!parserParseExpression(self, &bonsai, &expectStack, lex, 1)) {
      prosOutput_reportError(
         parserGetToken(lex).loc,
         "Expected expression.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }

   prosVector_pushBack(&expectStack, &(prosTokenTok){PROS_TOK_PUNCT_COLON});
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected ':'.",
         "Any or statement should be followed by a block."
      );
      ret = false;
      goto doReturn;
   }

   if (
      !prosSemantics_actOnOrStatement(&self->sem, stmt, &bonsai) ||
      !parserParseOperationScope(self, lex)
   ) {
      ret = false;
      goto doReturn;
   }

doReturn:
   prosVector_del(&expectStack);
   prosVector_del(&bonsai);
   return ret;
}

static bool parserParseElseStatement(prosParser *self, prosLexer *lex) {
   bool ret = true;
   prosToken tok, stmt = parserGetAndPopToken(lex);
   assert(stmt.tok == PROS_TOK_KEYWORD_ELSE && "Not a else keyowrd token.");

   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);

   prosVector_pushBack(&expectStack, &(prosTokenTok){PROS_TOK_PUNCT_COLON});
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected ':'.",
         "Any else statement should be followed by a block."
      );
      ret = false;
      goto doReturn;
   }

   if (
      !prosSemantics_actOnElseStatement(&self->sem, stmt) ||
      !parserParseOperationScope(self, lex)
   ) {
      ret = false;
      goto doReturn;
   }

doReturn:
   prosVector_del(&expectStack);
   return ret;
}

static bool parserParseWhileStatement(prosParser *self, prosLexer *lex) {
   bool ret = true;
   prosToken tok, stmt = parserGetAndPopToken(lex);
   assert(stmt.tok == PROS_TOK_KEYWORD_WHILE && "Not a while keyword token.");

   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);
   prosVector bonsai = prosVector_new(
      sizeof(prosSemanticsExprBonsaiNode),
      PROS_MAKE_DEL(prosSemanticsExprBonsaiNode_del)
   );

   if (!parserParseExpression(self, &bonsai, &expectStack, lex, 0)) {
      tok = parserGetToken(lex);
      prosOutput_reportError(
         tok.loc,
         "Expected expression.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }

   prosVector_pushBack(&expectStack, &(prosTokenTok){PROS_TOK_PUNCT_COLON});
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected ':' followed by any an operation block.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }

   if (
      !prosSemantics_actOnWhileStatement(&self->sem, stmt, &bonsai) ||
      !parserParseOperationScope(self, lex)
   ) {
      ret = false;
      goto doReturn;
   }

doReturn:
   prosVector_del(&bonsai);
   prosVector_del(&expectStack);
   return ret;
}

static bool parserParseLoopStatement(prosParser *self, prosLexer *lex) {
   bool ret = true;
   prosToken tok, stmt = parserGetAndPopToken(lex);
   assert(stmt.tok == PROS_TOK_KEYWORD_LOOP && "Not a loop keyword token.");

   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);

   prosVector_pushBack(&expectStack, &(prosTokenTok){PROS_TOK_PUNCT_COLON});
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected ':'.",
         "Any loop statement should be followed by a block."
      );
      ret = false;
      goto doReturn;
   }

   if (
      !prosSemantics_actOnLoopStatement(&self->sem, stmt) ||
      !parserParseOperationScope(self, lex)
   ) {
      ret = false;
      goto doReturn;
   }

doReturn:
   prosVector_del(&expectStack);
   return ret;
}

static bool parserParseExpressionStatement(
   prosParser *self,
   prosLexer *lex
) {
   bool ret = true;
   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);
   prosVector exprBonsai = prosVector_new(
      sizeof(prosSemanticsExprBonsaiNode),
      PROS_MAKE_DEL(prosSemanticsExprBonsaiNode_del)
   );

   if (!parserParseExpression(self, &exprBonsai, &expectStack, lex, 1)) {
      prosOutput_reportError(
         parserGetToken(lex).loc,
         "Expected expression.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }

   prosToken tok;
   if (!parserExpectationMatch(self, nullptr, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected the end of line.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }
   parserGetAndPopToken(lex);

   if (!prosSemantics_actOnExpressionStatement(
          &self->sem,
          tok,
          &exprBonsai
       )) {
      ret = false;
      goto doReturn;
   }

doReturn:
   prosVector_del(&exprBonsai);
   prosVector_del(&expectStack);
   return ret;
}

static bool parserParseBreakStatement(
   prosParser *self,
   prosLexer *lex
) {
   prosToken stmt = parserGetAndPopToken(lex);
   assert(stmt.tok == PROS_TOK_KEYWORD_BREAK && "Not a break keyword token.");

   prosToken tok;
   if (!parserExpectationMatch(self, nullptr, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected the end of line.",
         nullptr
      );
      return false;
   }

   if (!prosSemantics_actOnBreakStatement(&self->sem, stmt))
      return false;

   return true;
}

static bool parserParseContinueStatement(
   prosParser *self,
   prosLexer *lex
) {
   prosToken stmt = parserGetAndPopToken(lex);
   assert(stmt.tok == PROS_TOK_KEYWORD_CONTINUE && "Not a continue keyword token.");

   prosToken tok;
   if (!parserExpectationMatch(self, nullptr, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected the end of line.",
         nullptr
      );
      return false;
   }

   if (!prosSemantics_actOnContinueStatement(&self->sem, stmt))
      return false;

   return true;
}

static bool parserParseReturnStatement(
   prosParser *self,
   prosLexer *lex
) {
   bool ret = true;
   prosToken tok, stmt = parserGetAndPopToken(lex);
   assert(stmt.tok == PROS_TOK_KEYWORD_RETURN && "Not a return keyword token.");

   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);
   prosVector exprBonsai = prosVector_new(
      sizeof(prosSemanticsExprBonsaiNode),
      PROS_MAKE_DEL(prosSemanticsExprBonsaiNode_del)
   );

   if (!parserParseExpression(self, &exprBonsai, &expectStack, lex, 1)) {
      tok = parserGetToken(lex);
      prosOutput_reportError(
         tok.loc,
         "Expected expression.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }

   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected the end of line.",
         nullptr
      );
      ret = false;
      goto doReturn;
   }

   if (!prosSemantics_actOnReturnStatement(&self->sem, stmt, &exprBonsai)) {
      ret = false;
      goto doReturn;
   }

doReturn:
   prosVector_del(&exprBonsai);
   prosVector_del(&expectStack);
   return ret;
}

static bool parserParseOperationScope(
   prosParser *self,
   prosLexer *lex
) {
   bool ret = true;
   prosToken tok = parserGetToken(lex);
   prosVector expectStack = prosVector_new(sizeof(prosTokenTok), nullptr);

   prosVector_pushBack(&expectStack, &(prosTokenTok){PROS_TOK_INDENT});
   if (!parserExpectationMatch(self, &expectStack, lex, &tok)) {
      prosOutput_reportError(
         tok.loc,
         "Expected indentation as begining of operation.",
         "Blocks must be indented."
      );
      ret = false;
      goto finalize;
   }
   prosSemantics_checkScopeBounds(&self->sem, tok);

   while (lex->tokCount) {
      while (true) {
         tok = parserGetToken(lex);

         switch (tok.tok) {
         case PROS_TOK_NONE:
            pros_panic(
               "prosParser:"
               " Main loop received an invalid token: PROS_TOK_NONE."
               " While parsing operation block."
            );

         case PROS_TOK_EOF:  // We want that the main loop treat it.
            parserGetAndPopToken(lex);
            goto finalize;
         case PROS_TOK_EOL:
            parserGetAndPopToken(lex);
            goto endOfLine;

         case PROS_TOK_INDENT:
            // This should not happen.
            pros_panic("Unexpected indent/dedent token.");
         case PROS_TOK_DEDENT:
            parserGetAndPopToken(lex);
            goto finalize;

         case PROS_TOK_KEYWORD_VAR:
            parserParseObjectDecl(self, lex, false);
            break;
         case PROS_TOK_KEYWORD_VAL:
            parserParseObjectDecl(self, lex, true);
            break;

         case PROS_TOK_KEYWORD_IF:
            parserParseIfStatement(self, lex);
            break;
         case PROS_TOK_KEYWORD_OR:
            parserParseOrStatement(self, lex);
            break;
         case PROS_TOK_KEYWORD_ELSE:
            parserParseElseStatement(self, lex);
            break;
         case PROS_TOK_KEYWORD_WHILE:
            parserParseWhileStatement(self, lex);
            break;
         case PROS_TOK_KEYWORD_LOOP:
            parserParseLoopStatement(self, lex);
            break;
         case PROS_TOK_KEYWORD_BREAK:
            parserParseBreakStatement(self, lex);
            break;
         case PROS_TOK_KEYWORD_CONTINUE:
            parserParseContinueStatement(self, lex);
            break;
         case PROS_TOK_KEYWORD_RETURN:
            parserParseReturnStatement(self, lex);
            break;

         default:
            parserParseExpressionStatement(self, lex);
         }

         if (self->lex.tokp >= lex->tokCount)
            break;
      }

endOfLine:
      prosLexer_getLine(&self->lex);
   }

finalize:
   prosSemantics_actOnOperationScope(&self->sem, tok);
   prosVector_del(&expectStack);
   return ret;
}

bool prosParser_parseAll(prosParser *self) {
   prosSemantics_initModuleScope(&self->sem, "main");

   prosToken tok = {};
   prosLexer_getLine(&self->lex);

   while (self->lex.tokp < self->lex.tokCount) {
      while (true) {
         tok = parserGetAndPopToken(&self->lex);

         switch (tok.tok) {
         case PROS_TOK_NONE:
            pros_panic(
               "prosParser_parseAll():"
               " Main loop received an invalid token: PROS_TOK_NONE."
            );

         case PROS_TOK_EOF:
            goto endOfFile;
         case PROS_TOK_EOL:
            goto endOfLine;

         case PROS_TOK_INDENT:
         case PROS_TOK_DEDENT:
            // This should not happen.
            pros_panic("Unexpected indent/dedent token.");

            /* Declarations */

         // Function declaration.
         case PROS_TOK_KEYWORD_FUNC:
            parserParseFuncDecl(self, &self->lex);
            break;

         default:
            prosOutput_reportError(
               tok.loc,
               "Unexpected token.",
               nullptr
            );
            return false;
         }

         if (self->lex.tokp >= self->lex.tokCount)
            break;
      }

endOfLine:
      prosLexer_getLine(&self->lex);
   }

endOfFile:
   prosSemantics_actOnEndOfFile(&self->sem, tok);
   return true;
}
