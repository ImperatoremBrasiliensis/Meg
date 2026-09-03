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

void mtype_print(struct mtype *t, int ind) {
   indent(ind);

   switch (t->kind) {
   case mTYPE_INVAL:
      puts("Type is Invalid");
      break;
   case mTYPE_IDENT:
      printf(
         "TypeIdent %s, mut: %s\n",
         t->as.ident.name,
         t->mut ?
            "true" :
            "false"
      );
      break;
   case mTYPE_STRUCT:
      printf(
         "TypeStruct, mut: %s {\n",
         t->mut ? "true" : "false"
      );
      mdecl_print(t->as.struc.fields, ind + 1);
      indent(ind);
      puts("}");
      break;
   case mTYPE_ARRAY:
      printf("TypeArray, mut: %s {\n", t->mut ? "true" : "false");
      mexpr_print(t->as.array.size, ind + 1);
      mtype_print(t->as.array.type, ind + 1);
      indent(ind);
      puts("}");
      break;
   case mTYPE_SLICE:
      printf("TypeSlice, mut: %s {\n", t->mut ? "true" : "false");
      mtype_print(t->as.slice.szty, ind + 1);
      indent(ind + 1);
      puts("}");
      break;
   }
}

void mdecl_print(struct mdecl *d, int ind) {
   indent(ind);

   switch (d->kind) {
   case mDECL_INVAL:
      puts("Decl is Invalid");
      break;
   case mDECL_FUNC:
      printf("DeclFunc '%s' -> \n", d->id);
      if (d->type) {
         mtype_print(d->type, ind + 1);
      }
      if (d->as.func.params) {
         mdecl_print(d->as.func.params, ind + 1);
      }

      if (d->as.func.expr) {
         mexpr_print(d->as.func.expr, ind + 1);
         indent(ind);
      }

      puts("}");
      break;
   case mDECL_OBJ:
      printf("DeclObj '%s'", d->id);
      if (d->type) {
         puts(" {");
         mtype_print(d->type, ind + 1);
         indent(ind);
         puts("}");
      } else {
         puts("");
      }
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
      case mBIN_OP_EQL:
         name = "==";
         break;
      case mBIN_OP_NEQ:
         name = "!=";
         break;
      case mBIN_OP_GTR:
         name = ">";
         break;
      case mBIN_OP_LSS:
         name = "<";
         break;
      case mBIN_OP_GEQ:
         name = ">=";
         break;
      case mBIN_OP_LEQ:
         name = "<=";
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
      if (e->as.call.args) {
         mexpr_print(e->as.call.args, ind + 1);
      }
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
