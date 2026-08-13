/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Analysis/CFG.h>

#include <internal/Code/SourceLocation.h>
#include <internal/Orbita.h>
#include <internal/Output.h>
#include <internal/Utilities.h>

#include <assert.h>

struct cfgBranch {
   struct cfgBranch *next;
   struct cfgBranch *children;
   struct cfgBranch *lastChild;
   prosASTNode *node;
   enum cfgBranchType {
      CFG_BRANCH_INVAL = 0,
      CFG_BRANCH_FUNC_OPE,
      CFG_BRANCH_IF,
      CFG_BRANCH_IF_THEN_OPE,
      CFG_BRANCH_IF_OR_OPE,
      CFG_BRANCH_IF_ELSE_OPE,
      CFG_BRANCH_WHILE_OPE,
      CFG_BRANCH_LOOP_OPE
   } type;
   bool returns;
};

static struct cfgBranch *cfgNewBranch(
   struct cfgBranch *current,
   enum cfgBranchType type,
   prosASTNode *node,
   prosArena *arena
) {
   struct cfgBranch *bra = prosArena_alloc(arena, sizeof(struct cfgBranch));
   *bra = (struct cfgBranch){
      .node = node,
      .type = type
   };

   if (current) {
      if (current->children) {
         current->lastChild->next = bra;
         current->lastChild = bra;
      } else {
         current->children = bra;
         current->lastChild = bra;
      }
   }

   return bra;
}

[[maybe_unused]]
static void cfgPrintBranch(struct cfgBranch *branch, bool roots[], size_t *depth) {
   size_t i = 0;

   while (i < *depth) {
      if (roots[i])
         fputs("\033[37m \u2502 \033[0m", stdout);
      else
         fputs("   ", stdout);

      i++;
   }

   if (roots[i])
      fputs("\033[37m \u251c\u2574\033[0m", stdout);
   else
      fputs("\033[37m \u2570\u2574\033[0m", stdout);

   prosString type = nullptr;
   switch (branch->type) {
   case CFG_BRANCH_INVAL:
      type = "CFG_BRANCH_INVAL";
      break;
   case CFG_BRANCH_FUNC_OPE:
      type = "CFG_BRANCH_FUNC_OPE";
      break;
   case CFG_BRANCH_IF:
      type = "CFG_BRANCH_IF";
      break;
   case CFG_BRANCH_IF_THEN_OPE:
      type = "CFG_BRANCH_IF_THEN_OPE";
      break;
   case CFG_BRANCH_IF_OR_OPE:
      type = "CFG_BRANCH_IF_OR_OPE";
      break;
   case CFG_BRANCH_IF_ELSE_OPE:
      type = "CFG_BRANCH_IF_ELSE_OPE";
      break;
   case CFG_BRANCH_LOOP_OPE:
      type = "CFG_BRANCH_LOOP_OPE";
      break;
   case CFG_BRANCH_WHILE_OPE:
      type = "CFG_BRANCH_WHILE_OPE";
      break;
   }

   printf("[%s, returns: %s]\n", type, branch->returns ? "true" : "false");

   ++*depth;
   auto child = branch->children;
   while (child) {
      roots[*depth] = child->next != nullptr;
      cfgPrintBranch(child, roots, depth);

      child = child->next;
   }

   --*depth;
}

[[maybe_unused]]
static void cfgPrintGraph(struct cfgBranch *root, size_t count) {
   printf("Control Flow Graph:\n");
   size_t depth = 0;
   bool roots[count];

   roots[0] = false;
   cfgPrintBranch(root, roots, &depth);
}

static bool cfgAnalyzeOperation(
   prosASTNode *node,
   struct cfgBranch *branch,
   prosArena *arena,
   prosSourceLocation *loc
);

static bool cfgAnalyzeIfStatement(
   prosASTNode *node,
   struct cfgBranch *branch,
   prosArena *arena,
   prosSourceLocation *loc
) {
   assert(node->type == PROS_AST_IF_STATEMENT && "Not an if statement.");

   bool ret = true, elseReturns = false;
   auto ifnode = &node->nodeAs.ifStatement;
   auto ifbra = cfgNewBranch(branch, CFG_BRANCH_IF_THEN_OPE, node, arena);
   ret = cfgAnalyzeOperation(ifnode->then, ifbra, arena, loc) ?
      ret :
      false;

   if (ifnode->ifType == PROS_AST_IF_STATEMENT_IF_OR) {
      auto orbra = cfgNewBranch(branch, CFG_BRANCH_IF_OR_OPE, ifnode->elseBlock, arena);
      ret = cfgAnalyzeIfStatement(ifnode->elseBlock, orbra, arena, loc) ?
         ret :
         false;

      elseReturns = orbra->returns;
   } else if (ifnode->ifType == PROS_AST_IF_STATEMENT_IF_ELSE) {
      auto elsebra = cfgNewBranch(branch, CFG_BRANCH_IF_ELSE_OPE, node, arena);
      ret = cfgAnalyzeOperation(ifnode->elseBlock, elsebra, arena, loc) ?
         ret :
         false;

      elseReturns = elsebra->returns;
   }

   if (ifbra->returns && elseReturns) {
      branch->returns = true;
   }
   return ret;
}

static bool cfgAnalyzeOperation(
   prosASTNode *node,
   struct cfgBranch *branch,
   prosArena *arena,
   prosSourceLocation *loc
) {
   assert(node->type == PROS_AST_SCOPE_OPERATION && "Not an operation.");

   bool ret = true;
   auto nav = prosASTListNavigator_new(node->nodeAs.operation.body);
   prosASTNode *child = nullptr;

   while ((child = prosASTListNavigator_next(&nav))) {
      switch (child->type) {
      case PROS_AST_IF_STATEMENT:
         auto ifbra = cfgNewBranch(branch, CFG_BRANCH_IF, child, arena);
         ret = cfgAnalyzeIfStatement(child, ifbra, arena, loc) ? ret : false;

         if (ifbra->returns) {
            branch->returns = true;
         }
         break;

      case PROS_AST_WHILE_STATEMENT:
         auto whilenode = &child->nodeAs.whileStatement;
         auto whilebra = cfgNewBranch(branch, CFG_BRANCH_WHILE_OPE, child, arena);
         ret = cfgAnalyzeOperation(whilenode->doBlock, whilebra, arena, loc) ?
            ret :
            false;
         break;

      case PROS_AST_LOOP_STATEMENT:
         auto loopnode = &child->nodeAs.loopStatement;
         auto loopbra = cfgNewBranch(branch, CFG_BRANCH_LOOP_OPE, child, arena);
         ret = cfgAnalyzeOperation(loopnode->block, loopbra, arena, loc) ?
            ret :
            false;
         break;

      case PROS_AST_BREAK_STATEMENT:
         if (
            branch->node->type == PROS_AST_WHILE_STATEMENT ||
            branch->node->type == PROS_AST_FOR_STATEMENT ||
            branch->node->type == PROS_AST_LOOP_STATEMENT ||
            branch->node->type == PROS_AST_REPEAT_STATEMENT
         ) {
            goto endOfBranch;
         } else {
            pros_panic("prosCFG: Unexpected `break` statement.");
         }

      case PROS_AST_CONTINUE_STATEMENT:
         if (
            branch->node->type == PROS_AST_WHILE_STATEMENT ||
            branch->node->type == PROS_AST_FOR_STATEMENT ||
            branch->node->type == PROS_AST_LOOP_STATEMENT ||
            branch->node->type == PROS_AST_REPEAT_STATEMENT
         ) {
            goto endOfBranch;
         } else {
            pros_panic("prosCFG: Unexpected `continue` statement.");
         }

      case PROS_AST_RETURN_STATEMENT:
         branch->returns = true;
         goto endOfBranch;

      default:
      }

      if (branch->returns) {
         goto endOfBranch;
      }
   }

endOfBranch:
   if (branch->type == CFG_BRANCH_FUNC_OPE && !branch->returns) {
      auto retType = prosAST_getTypeDecl(branch->node->nodeAs.funcDecl.retType);

      if (!retType.isBuiltin || retType.as.builtinp != &PROS_VOID_TYPE) {
         prosOutput_reportError(
            *loc,
            "Non-void function should return a value.",
            nullptr
         );
         ret = false;
      }
   }

   return ret;
}

bool prosCFG_analyzeFunc(
   prosASTNode *funcNode,
   prosAllocator *altor,
   prosSourceLocation loc
) {
   assert(funcNode->type == PROS_AST_DECL_FUNC && "Not a function declaration node.");

   bool ret = true;
   prosArena arena = prosArena_new(altor);
   auto funcbra = cfgNewBranch(
      nullptr,
      CFG_BRANCH_FUNC_OPE,
      funcNode,
      &arena
   );

   ret = cfgAnalyzeOperation(funcNode->nodeAs.funcDecl.body, funcbra, &arena, &loc);

   prosArena_del(&arena);
   return ret;
}
