/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#pragma once

#include <megc/loc.h>

#include <stdint.h>

/* Unit. */

struct munit {
   const char *name;
   struct mdecl *decls;
};

/* Concepts. */

struct mtype {
   const char *name;
   bool mut;
};

/* Declarations. */

enum mdecl_kind {
   mDECL_INVAL = 0,
   mDECL_FUNC,
   mDECL_OBJ
};

struct mdecl {
   struct mdecl *next;
   struct mloc loc;
   const char *id;
   struct mtype types;
   union {
      struct mtype_decl {
         struct mdecl *decls;
      } type;

      struct mfunc_decl {
         struct mdecl *params;
         struct mexpr *expr;
      } func;
   } as;
   enum mdecl_kind kind;
};

/* Expressions. */

enum mexpr_kind {
   mEXPR_INVAL = 0,
   mEXPR_BIN_OP,
   mEXPR_UNA_OP,
   mEXPR_DECL_REF,
   mEXPR_CALL,
   mEXPR_LIT,
   mEXPR_PAREN
};

enum mbin_op_kind {
   mBIN_OP_INVAL = 0,
   mBIN_OP_ADD,
   mBIN_OP_SUB,
   mBIN_OP_MUL,
   mBIN_OP_DIV,
   mBIN_OP_MOD,
   mBIN_OP_AND,
   mBIN_OP_BOR,
   mBIN_OP_EOR,
   mBIN_OP_LAND,
   mBIN_OP_LOR
};

enum muna_op_kind {
   mUNA_OP_INVAL = 0,
   mUNA_OP_PLUS,
   mUNA_OP_MINUS,
   mUNA_OP_NEG
};

enum mlit_kind {
   mLIT_INVAL = 0,
   mLIT_INTEGER,
   mLIT_FLOAT,
   mLIT_BOOL,
   mLIT_STRING,
   mLIT_CHAR
};

struct mexpr {
   struct mexpr *next;  // Used only in lists.
   struct mloc loc;
   union {
      struct mbin_op {
         struct mexpr *lhs, *rhs;
         enum mbin_op_kind kind;
      } bin_op;

      struct muna_op {
         struct mexpr *oprnd;
         enum muna_op_kind kind;
      } una_op;

      struct mdecl_ref {
         const char *declid;
      } decl_ref;

      struct mcall {
         struct mexpr *decl;  // Any callable type expr.
         struct mexpr *args;
      } call;

      struct mlit {
         const char *buf;
         enum mlit_kind kind;
      } lit;

      struct mparen {
         struct mexpr *child;
      } paren;
   } as;
   enum mexpr_kind kind;
};

/* Statements. */

struct mstmt {
   struct mstmt *next;
   struct mloc loc;
   union {
      struct mdef {
         const char *id;
      } def;

      struct massign {
         uint64_t id;
      } assign;

      struct mnew {
         const char *id;
         uint64_t objid;
      } new;

      struct mdel {
         int _;
      } del;

      struct mresult {
         int _;
      } result;
   } as;
};

void munit_print(struct munit *u);
void mdecl_print(struct mdecl *e, int ind);
void mexpr_print(struct mexpr *e, int ind);
