/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/AST/ASTContext.h>

#include <internal/Orbita.h>

#include <assert.h>

prosASTContext prosASTContext_new(prosAST *ast) {
   assert(ast && "`ast` parameter is invalid.");

   prosASTContext ret = {
      .ast = ast,
      .ctxStack = malloc(4 * sizeof(prosASTNode *)),
      .ctxStackSize = 4
   };

   return ret;
}

void prosASTContext_del(prosASTContext *self) {
   free(self->ctxStack);
   *self = (prosASTContext){};
}

bool prosASTContext_traverseDown(prosASTContext *self, prosASTNode *node) {
   assert(node && "Passed `nullptr` as `node` parameter.");
   assert(node->type != PROS_AST_INVALID_NODE && "Received an invalid node.");

   if (self->ctxDepth == self->ctxStackSize) {
      self->ctxStackSize *= 2;
      self->ctxStack = realloc(
         self->ctxStack,
         self->ctxStackSize * sizeof(prosASTNode *)
      );

      if (!self->ctxStack) {
         pros_panic(
            "prosASTContext_traverseDown():"
            "Call to `realloc()` resulted in error."
         );
      }
   }

   self->ctxStack[self->ctxDepth++] = node;
   return true;
}

bool prosASTContext_traverseUp(prosASTContext *self) {
   if (self->ctxDepth <= 1) {
      pros_panic(
         "prosASTContext_traverseUp():"
         " Attempt to traverse up the 1st node of the context."
      );
   }

   self->ctxDepth--;
   return true;
}

prosASTNode *prosASTContext_getCurrentNode(prosASTContext *self) {
   assert(self->ctxDepth && "Context stack is empty.");

   return self->ctxStack[self->ctxDepth - 1];
}
