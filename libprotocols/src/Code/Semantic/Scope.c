/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Semantic/Scope.h>

#include <internal/Orbita.h>
#include <internal/Output.h>

#include <assert.h>
#include <string.h>

prosScope prosScope_new(
   prosASTBuilder *astBuilder,
   prosASTNode *node,
   prosASTNode *owner,
   prosAllocator *dataAllocator
) {
   assert(node != nullptr && "Passed `nullptr` as 'node' parameter.");
   assert(node != owner && "'owner' and 'node' should not point to the same node.");

   prosScopeType type = PROS_SCOPE_UNDEFINED;
   switch (node->type) {
   case PROS_AST_SCOPE_MODULE:
      assert(
         owner == nullptr &&
         "'owner' parameter should be `nullptr` for a module scope."
      );
      type = PROS_SCOPE_MODULE;
      break;

   case PROS_AST_SCOPE_OPERATION:
      assert(
         owner != nullptr &&
         "Passed `nullptr` as 'owner' parameter for an operation scope."
      );
      type = PROS_SCOPE_OPERATION;
      break;

   case PROS_AST_INVALID_NODE:
      pros_panic(
         "prosScope_new(): Passed an invalid node as 'node' parameter."
      );

   default:
      pros_panic(
         "prosScope_new(): Passed a non-scope AST node as 'node' parameter."
      );
   }

   prosScope ret = {};
   ret.node = node;
   ret.owner = owner;
   ret.builder = astBuilder;
   ret.indentation = -1;
   ret.allocator = dataAllocator;
   ret.type = type;

   constexpr size_t INITIAL_DECL_TABLE_SIZE = 16;
   ret.declTableSize = INITIAL_DECL_TABLE_SIZE;
   ret.declList = nullptr;
   ret.declTable = calloc(
      INITIAL_DECL_TABLE_SIZE,
      sizeof(prosScopeDeclMapBucket)
   );
   return ret;
}

void prosScope_del(prosScope *self) {
   // Free all memory allocated.
   auto decl = self->declList;
   while (decl) {
      prosAllocator_free(self->allocator, decl->decl);
      decl = decl->next;
   }
   free(self->declTable);

   switch (self->type) {
   case PROS_SCOPE_OPERATION:
      // Traverse up the scope node.
      prosASTContext_traverseUp(&self->builder->ctx);
      // Traverse up the statement or declaration node.
      prosASTContext_traverseUp(&self->builder->ctx);
      break;

   case PROS_SCOPE_UNDEFINED:
      pros_panic("prosScope: Attempt to destroy a scope of unknown type.");

   default:
   }

   *self = (prosScope){};
}

static void scopeInsertDecl(prosScope *self, prosScopeDecl *decl) {
   uint64_t hash = decl->id.data.id->hash;
   size_t pos = hash & (self->declTableSize - 1);
   prosScopeDeclMapBucket new = {
      .next = self->declList,
      .hash = hash,
      .psl = 0,
      .decl = decl
   };

   while (true) {
      auto bucket = &self->declTable[pos];

      if (!bucket->decl) {
         *bucket = new;
         self->declList = bucket;
         break;
      }

      if (bucket->psl < new.psl) {
         // Swap the buckets.
         auto temp = *bucket;
         *bucket = new;
         bucket->next = temp.next;

         new = temp;
         new.next = self->declList;
      }

      pos = (pos + 1) & self->declTableSize - 1;
      new.psl++;
   }

   self->declCount++;
}

static void scopeDoRehash(prosScope *self) {
   auto oldDeclTable = self->declTable;
   size_t oldTableSize = self->declTableSize;

   self->declTableSize = self->declTableSize * 2;
   self->declList = nullptr;
   self->declTable = calloc(
      self->declTableSize,
      sizeof(prosScopeDeclMapBucket)
   );

   self->declCount = 0;
   for (size_t i = 0; i < oldTableSize; i++)
      if (oldDeclTable[i].decl)
         scopeInsertDecl(self, oldDeclTable[i].decl);

   free(oldDeclTable);
}

bool prosScope_pushDecl(prosScope *self, prosScopeDecl *decl) {
   if (!decl)
      pros_panic("prosScope_pushDecl(): `nullptr` passed as `decl` argument.");

   decl->isUsed = false;
   decl->scope = self;

   prosScopeDecl *new = prosAllocator_alloc(self->allocator, sizeof(prosScopeDecl));
   assert(new != (prosScopeDecl *) 0x0000003000039d50);
   *new = *decl;
   scopeInsertDecl(self, new);

   if ((double) self->declCount / self->declTableSize > 0.70)
      scopeDoRehash(self);
   return false;
}

prosScopeDecl *prosScope_findDecl(prosScope *self, uint64_t hash) {
   size_t pos = hash & (self->declTableSize - 1);
   size_t psl = 0;

   while (true) {
      auto bucket = self->declTable[pos];

      if (!bucket.hash || bucket.psl < psl)
         return nullptr;

      if (bucket.hash == hash)
         return bucket.decl;

      pos = (pos + 1) & (self->declTableSize - 1);
      psl++;
   }
}
