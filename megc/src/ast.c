/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <megc/ast.h>

#include <stdio.h>

static void indent(int ind) {
   while (ind--) {
      printf("   ");
   }
}

void munit_print(struct munit *u) {
   printf("Unit '%s' {\n", u->name);
   mdecl_print(u->decls, 1);
   puts("}");
}

void mdecl_print(struct mdecl *d, int ind) {
   indent(ind);

   switch (d->kind) {
   case mDECL_INVAL:
      puts("Decl is Invalid");
      break;
   case mDECL_FUNC:
      printf("DeclFunc '%s' -> '%s' {", d->id, d->types.name);
      if (d->as.func.params) {
         puts("");
         mdecl_print(d->as.func.params, ind + 1);
         indent(ind);
      }

      if (d->as.func.expr) {
         puts("");
         mexpr_print(d->as.func.expr, ind + 1);
         indent(ind);
         puts("}");
      } else {
         puts("}");
      }
      break;
   case mDECL_OBJ:
      printf("DeclObj '%s': %s\n", d->id, d->types.name);
      break;
   }

   if (d->next) {
      mdecl_print(d->next, ind);
   }
}

void mexpr_print(struct mexpr *e, int ind) {
   const char *name = 0;

   indent(ind);

   switch (e->kind) {
   case mEXPR_INVAL:
      puts("Expr is invalid");
      break;
   case mEXPR_BIN_OP:
      switch (e->as.bin_op.kind) {
      case mBIN_OP_INVAL:
         puts("ExpxBinOp is invalid");
         goto end;
      case mBIN_OP_ADD:
         name = "+";
         break;
      case mBIN_OP_SUB:
         name = "-";
         break;
      case mBIN_OP_MUL:
         name = "*";
         break;
      case mBIN_OP_DIV:
         name = "/";
         break;
      case mBIN_OP_MOD:
         name = "%";
         break;
      case mBIN_OP_AND:
         name = "&";
         break;
      case mBIN_OP_BOR:
         name = "|";
         break;
      case mBIN_OP_EOR:
         name = "^";
         break;
      case mBIN_OP_LAND:
         name = "&&";
         break;
      case mBIN_OP_LOR:
         name = "||";
         break;
      }
      printf("ExprBinOp %s {\n", name);
      mexpr_print(e->as.bin_op.lhs, ind + 1);
      mexpr_print(e->as.bin_op.rhs, ind + 1);
      indent(ind);
      puts("}");
      break;
   case mEXPR_UNA_OP:
      switch (e->as.una_op.kind) {
      case mUNA_OP_INVAL:
         puts("ExprUnaOp is invalid");
         goto end;
      case mUNA_OP_PLUS:
         name = "+";
         break;
      case mUNA_OP_MINUS:
         name = "-";
         break;
      case mUNA_OP_NEG:
         name = "!";
         break;
      }
      printf("ExprUnaOp %s {\n", name);
      mexpr_print(e->as.una_op.oprnd, ind + 1);
      indent(ind);
      puts("}");
      break;
   case mEXPR_DECL_REF:
      printf("ExprDeclRef '%s'\n", e->as.decl_ref.declid);
      break;
   case mEXPR_CALL:
      puts("ExprCall {");
      mexpr_print(e->as.call.decl, ind + 1);
      mexpr_print(e->as.call.args, ind + 1);
      indent(ind);
      puts("}");
      break;
   case mEXPR_LIT:
      printf("ExprLit '%s'\n", e->as.lit.buf);
      break;
   case mEXPR_PAREN:
      printf("ExprParen {\n");
      mexpr_print(e->as.paren.child, ind + 1);
      indent(ind);
      puts("}");
      break;
   }

end:
   if (e->next) {
      mexpr_print(e->next, ind);
   }
}
