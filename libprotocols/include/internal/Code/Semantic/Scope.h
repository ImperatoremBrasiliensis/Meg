/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_PARSE_SCOPE_H
#define PROTOCOLS_INTERNAL_CODE_PARSE_SCOPE_H

#include <internal/Code/AST/AST.h>
#include <internal/Code/AST/ASTBuilder.h>
#include <internal/Code/Lexicon/IdTable.h>
#include <internal/Code/Lexicon/Token.h>
#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef enum prosScopeDeclType_s : uint8_t {
   PROS_SCOPE_DECL_UNDEFINED,
   PROS_SCOPE_DECL_FUNCTION,
   PROS_SCOPE_DECL_TYPEDEF,
   PROS_SCOPE_DECL_PARAMETER,
   PROS_SCOPE_DECL_VARIABLE,
   PROS_SCOPE_DECL_VALUE
} prosScopeDeclType;

typedef struct prosScopeDecl_s {
   struct prosScope_s *scope;
   prosToken id;
   prosScopeDeclType declType;
   prosASTNode *astNode;
   bool isUsed;

   union {
      struct {
         prosASTType typeIndex;
      } func;
      struct {
         prosASTType type;
      } varDecl;
      struct {
         prosASTType type;
      } valDecl;
      struct {
         prosASTType typeIndex;
      } param;
   } dataAs;
} prosScopeDecl;

void prosScopeDecl_del(prosScopeDecl *self);

typedef enum prosScopeType_e : uint8_t {
   PROS_SCOPE_UNDEFINED,
   PROS_SCOPE_MODULE,
   PROS_SCOPE_OPERATION
} prosScopeType;

typedef struct prosScopeDeclMapBucket_s {
   uint64_t hash, psl;
   struct prosScopeDeclMapBucket_s *next;
   prosScopeDecl *decl;
} prosScopeDeclMapBucket;

typedef struct prosScope_s {
   prosASTBuilder *builder;
   prosASTNode *node, *owner;
   prosScopeDeclMapBucket *declTable, *declList;
   prosAllocator *allocator;
   uint32_t indentation;
   uint32_t declTableSize, declCount;
   prosScopeType type;
} prosScope;

prosScope prosScope_new(
   prosASTBuilder *astBuilder,
   prosASTNode *node,
   prosASTNode *owner,
   prosAllocator *dataAllocator
);

void prosScope_del(prosScope *self);

bool prosScope_pushDecl(prosScope *self, prosScopeDecl *decl);

prosScopeDecl *prosScope_findDecl(prosScope *self, uint64_t hash);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_CODE_PARSE_SCOPE_H
