/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <megc/ast.h>
#include <megc/diagno.h>
#include <megc/lexer.h>
#include <megc/parser.h>
#include <megc/token.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

struct parser {
   struct mlexer lex;
   struct mtoken fst, snd;
   const char *buf;
};

static struct mtoken advance(struct parser *self) {
   if (self->snd.kind != mTOK_INVAL) {
      self->fst = self->snd;
      self->snd = mlexer_lex(&self->lex);
   } else {
      self->fst = mlexer_lex(&self->lex);
      self->snd = mlexer_lex(&self->lex);
   }

   return self->fst;
}

/* Current token. */
static inline struct mtoken cur(struct parser *self) {
   return self->fst;
}

/* Skips mTOK_DOC and mTOK_EOL, if any. */
static inline struct mtoken nxtvalid(struct parser *self) {
   auto t = cur(self);
   while (t.kind != mTOK_EOF) {
      if (
         t.kind != mTOK_EOL &&
         t.kind != mTOK_DOC
      ) {
         break;
      }

      t = advance(self);
   }
   return t;
}

static void skipuntil(struct parser *self, enum mtoken_kind tok) {
   while (cur(self).kind != tok) {
      auto t = advance(self);
      if (t.kind == mTOK_EOF) {
         return;
      }
   }

   advance(self);
}

static inline bool expect(
   struct parser *self,
   struct mtoken *tok,
   enum mtoken_kind kind
) {
   *tok = nxtvalid(self);
   if (tok->kind == kind) {
      advance(self);
      return true;
   } else {
      return false;
   }
}

static inline bool eol(
   struct parser *self,
   struct mtoken *tok
) {
again:
   *tok = cur(self);
   if (tok->kind == mTOK_DOC) {
      advance(self);
      goto again;
   }

   if (tok->kind == mTOK_EOL) {
      advance(self);
      return true;
   } else {
      return false;
   }
}

/* Concepts. */

static struct mtype parse_type(struct parser *self) {
   struct mtoken tok;
   if (expect(self, &tok, mTOK_ID)) {
      return (struct mtype){
         .name = tok.lit,
         .mut = true
      };
   }

   return (struct mtype){
      .mut = true
   };
}

/* Expressions. */

struct expr {
   int prec;
   struct mexpr *(*nud)(
      struct parser *self,
      struct mtoken tok
   );
   struct mexpr *(*led)(
      struct parser *self,
      struct mtoken tok,
      struct mexpr *left
   );
};

static struct mexpr *parse_expr(struct parser *self, int prec);
static struct mexpr *parse_bin_op(
   struct parser *self,
   struct mtoken tok,
   struct mexpr *expr
);
static struct mexpr *parse_una_op(
   struct parser *self,
   struct mtoken tok
);
static struct mexpr *parse_decl_ref(
   struct parser *self,
   struct mtoken tok
);
static struct mexpr *parse_lit(
   struct parser *self,
   struct mtoken tok
);
static struct mexpr *parse_paren(
   struct parser *self,
   struct mtoken tok
);
static struct mexpr *parse_call(
   struct parser *self,
   struct mtoken tok,
   struct mexpr *expr
);

/*
 * Operator precedence:
 * 999: Literals and declaration references;
 * 100: `()` and `.`;
 *  90: `$`, `&`, `!`, and `+` and `-` unaries;
 *  80: `*`, `/` and `%`;
 *  70: `+` and `-` binaries;
 *  60: `<`, `>`, `<=`, `>=`, `==` and `!=`;
 *  50: `&` bitwise;
 *  40: `^`;
 *  30: `|`;
 *  20: `&&`;
 *  10: `||`;
 */
static struct expr NUD_OPS[mTOK_MAX] = {
   [mTOK_ADD] = {90, parse_una_op, nullptr},
   [mTOK_SUB] = {90, parse_una_op, nullptr},
   [mTOK_AND] = {90, parse_una_op, nullptr},
   [mTOK_NEG] = {90, parse_una_op, nullptr},

   [mTOK_LPAREN] = {100, parse_paren, nullptr},
   [mTOK_ID] = {999, parse_decl_ref, nullptr},
   [mTOK_INTEGER] = {999, parse_lit, nullptr},
};

static struct expr LED_OPS[mTOK_MAX] = {
   [mTOK_LOR] = {10, nullptr, parse_bin_op},

   [mTOK_LAND] = {20, nullptr, parse_bin_op},

   [mTOK_BOR] = {30, nullptr, parse_bin_op},

   [mTOK_EOR] = {40, nullptr, parse_bin_op},

   [mTOK_AND] = {50, nullptr, parse_bin_op},

   [mTOK_EQL] = {60, nullptr, parse_bin_op},
   [mTOK_GTR] = {60, nullptr, parse_bin_op},
   [mTOK_LSS] = {60, nullptr, parse_bin_op},

   [mTOK_ADD] = {70, nullptr, parse_bin_op},
   [mTOK_SUB] = {70, nullptr, parse_bin_op},

   [mTOK_MUL] = {80, nullptr, parse_bin_op},
   [mTOK_DIV] = {80, nullptr, parse_bin_op},
   [mTOK_MOD] = {80, nullptr, parse_bin_op},

   [mTOK_LPAREN] = {100, nullptr, parse_call},
};

static struct mexpr *parse_call(
   struct parser *self,
   struct mtoken tok,
   struct mexpr *expr
) {
   assert(expr->kind == mEXPR_DECL_REF);
   assert(tok.kind == mTOK_LPAREN);
   advance(self);

   struct mexpr *ret = malloc(sizeof *ret);
   *ret = (struct mexpr){
      .kind = mEXPR_CALL,
      .loc = tok.loc,
      .as.call = {
         .decl = expr
      }
   };

   struct mexpr *farg = nullptr;  // Fisrt arg.
   struct mexpr *larg = farg;     // Last arg.
   if (cur(self).kind != mTOK_RPAREN) {
      while (true) {
         if (cur(self).kind == mTOK_COMMA) {
            mferro(cur(self).loc, "Expected argument expression.");
            continue;
         }

         auto arg = parse_expr(self, 0);
         if (!farg) {
            farg = arg;
            larg = farg;
         } else {
            larg->next = arg;
            larg = arg;
         }

         struct mtoken tok;
         if (!expect(self, &tok, mTOK_COMMA)) {
            if (!expect(self, &tok, mTOK_RPAREN)) {
               mferro(tok.loc, "Expected ',' or ')' in argument list.");
               skipuntil(self, mTOK_RPAREN);
               goto inval;
            }
            break;
         }
      }
   }

   ret->as.call.args = farg;
   return ret;

inval:
   ret->kind = mEXPR_INVAL;
   return ret;
}

static struct mexpr *parse_paren(
   struct parser *self,
   struct mtoken tok
) {
   assert(tok.kind == mTOK_LPAREN);
   advance(self);

   if (cur(self).kind == mTOK_RPAREN) {
      mferro(tok.loc, "Expected expression.");
      return nullptr;
   }

   struct mexpr *expr = parse_expr(self, 0);

   struct mtoken rp;
   if (!expect(self, &rp, mTOK_RPAREN)) {
      mferro(rp.loc, "Expected ')'.");
      skipuntil(self, mTOK_RPAREN);
   }

   struct mexpr *ret = malloc(sizeof *ret);
   *ret = (struct mexpr){
      .kind = mEXPR_PAREN,
      .loc = tok.loc,
      .as.paren = {
         .child = expr
      }
   };

   return ret;
}

static struct mexpr *parse_decl_ref(
   struct parser *self,
   struct mtoken tok
) {
   assert(tok.kind == mTOK_ID);
   advance(self);

   struct mexpr *ret = malloc(sizeof *ret);
   *ret = (struct mexpr){
      .kind = mEXPR_DECL_REF,
      .loc = tok.loc,
      .as.decl_ref = {
         .declid = tok.lit
      }
   };

   return ret;
}

static struct mexpr *parse_lit(
   struct parser *self,
   struct mtoken tok
) {
   assert(tok.kind == mTOK_INTEGER);
   advance(self);

   struct mexpr *ret = malloc(sizeof *ret);
   *ret = (struct mexpr){
      .kind = mEXPR_LIT,
      .loc = tok.loc,
      .as.lit = {
         .kind = mLIT_INTEGER,
         .buf = tok.lit
      }
   };

   return ret;
}

static struct mexpr *parse_bin_op(
   struct parser *self,
   struct mtoken tok,
   struct mexpr *expr
) {
   advance(self);

   enum mbin_op_kind kind = mBIN_OP_INVAL;
   switch (tok.kind) {
   case mTOK_ADD:
      kind = mBIN_OP_ADD;
      break;
   case mTOK_SUB:
      kind = mBIN_OP_SUB;
      break;
   case mTOK_MUL:
      kind = mBIN_OP_MUL;
      break;
   case mTOK_DIV:
      kind = mBIN_OP_DIV;
      break;
   case mTOK_MOD:
      kind = mBIN_OP_MOD;
      break;
   default:
      madeus("Invalid binary operator.");
   }

   struct mexpr *ret = malloc(sizeof *ret);
   *ret = (struct mexpr){
      .kind = mEXPR_BIN_OP,
      .loc = tok.loc,
      .as.bin_op = {
         .kind = kind,
         .lhs = expr,
      }
   };

   int prec = LED_OPS[tok.kind].prec;
   ret->as.bin_op.rhs = parse_expr(self, prec);
   if (!ret->as.bin_op.rhs) {
      mferro(cur(self).loc, "Invalid operand.");
      goto inval;
   }

   return ret;

inval:
   ret->kind = mEXPR_INVAL;
   return ret;
}

static struct mexpr *parse_una_op(
   struct parser *self,
   struct mtoken tok
) {
   advance(self);

   enum muna_op_kind kind = mUNA_OP_INVAL;
   switch (tok.kind) {
   case mTOK_ADD:
      kind = mUNA_OP_PLUS;
      break;
   case mTOK_SUB:
      kind = mUNA_OP_MINUS;
      break;
   case mTOK_NEG:
      kind = mUNA_OP_NEG;
      break;
   default:
      madeus("Invalid unary operator.");
   }

   struct mexpr *ret = malloc(sizeof *ret);
   *ret = (struct mexpr){
      .kind = mEXPR_UNA_OP,
      .loc = tok.loc,
      .as.una_op = {
         .kind = kind
      }
   };

   int prec = NUD_OPS[tok.kind].prec;
   ret->as.una_op.oprnd = parse_expr(self, prec);
   if (!ret->as.una_op.oprnd) {
      mferro(cur(self).loc, "Invalid operand.");
      goto inval;
   }

   return ret;

inval:
   ret->kind = mEXPR_INVAL;
   return ret;
}

static struct expr nud(struct mtoken tok) {
   if (tok.kind < mTOK_MAX) {
      return NUD_OPS[tok.kind];
   }

   return (struct expr){};
}

static struct expr led(struct mtoken tok) {
   if (tok.kind < mTOK_MAX) {
      return LED_OPS[tok.kind];
   }

   return (struct expr){};
}

static struct mexpr *parse_expr(
   struct parser *self,
   int prec
) {
   auto tok = nxtvalid(self);
   struct expr left = nud(tok);
   if (!left.nud) {
      mferro(tok.loc, "Expected expression.");
      return nullptr;
   }

   struct mexpr *expr = left.nud(self, tok);

   while (true) {
      tok = cur(self);
      left = led(tok);
      if (left.prec > prec && left.led) {
         expr = left.led(self, tok, expr);
      } else {
         break;
      }
   }

   return expr;
}

/* Declarations. */

static struct mdecl *parse_objdecl(struct parser *self) {
   /*
    * Object declarations syntax
    * is as follows:
    * [ID][COLON] <type> [ASSIGN: optional] <expr>
    */

   auto tok = cur(self);
   assert(tok.kind == mTOK_ID && "Not an obj decl");

   struct mdecl *ret = malloc(sizeof *ret);
   *ret = (struct mdecl){
      .kind = mDECL_OBJ,
      .id = tok.lit
   };

   /* Skips the id and colon. */
   advance(self);
   if (!expect(self, &tok, mTOK_COLON)) {
      mferro(tok.loc, "Expected colon in object declaration.");
      goto inval;
   }

   ret->types = parse_type(self);
   return ret;

inval:
   ret->kind = mDECL_INVAL;
   return ret;
}

static struct mdecl *parse_initlist(
   struct parser *self,
   enum mtoken_kind ter  // Terminator.
) {
   /*
    * Initialization lists are
    * sequences of declarations
    * comma-separated. It may be
    * default initialized.
    */
   struct mtoken tok = cur(self);
   struct mdecl *fst = nullptr, *lst = fst;
   while (tok.kind == mTOK_ID) {
      auto obj = parse_objdecl(self);
      if (!fst) {
         fst = obj;
         lst = fst;
      } else {
         lst->next = obj;
         lst = obj;
      }

      if (lst->kind == mDECL_INVAL) {
         /* The state may be corrupted, skip. */
         skipuntil(self, ter);
         return fst;
      }

      if (expect(self, &tok, mTOK_COMMA)) {
         tok = nxtvalid(self);
         if (tok.kind == mTOK_ID) {
            continue;
         }

         mfwarn(tok.loc, "Dangling comma.");
      }
      break;
   }

   if (!expect(self, &tok, ter)) {
      mferro(self->fst.loc, "Expected ')'.");
      skipuntil(self, ter);
   }
   return fst;
}

static struct mdecl *parse_func(struct parser *self) {
   assert(
      self->fst.kind == mTOK_ID &&
      self->snd.kind == mTOK_LPAREN &&
      "Not a function decl"
   );

   auto tok = self->fst;

   struct mdecl *ret = malloc(sizeof *ret);
   *ret = (struct mdecl){
      .kind = mDECL_FUNC,
      .id = tok.lit
   };

   advance(self);
   tok = advance(self);

   /* Parses all the parameters if any. */
   if (expect(self, &tok, mTOK_RPAREN)) {
      advance(self);
   } else {
      ret->as.func.params =
         parse_initlist(self, mTOK_RPAREN);
   }

   tok = self->fst;
   if (tok.kind != mTOK_COLON) {
      mferro(tok.loc, "Expected ':' followed by the result type.");
      skipuntil(self, mTOK_EOL);
      goto inval;
   }
   advance(self);
   ret->types = parse_type(self);

   if (!eol(self, &tok)) {
      if (expect(self, &tok, mTOK_ASSIGN)) {
         ret->as.func.expr = parse_expr(self, 0);
      } else {
         mferro(tok.loc, "Expected the end of the line.");
         skipuntil(self, mTOK_EOL);
      }
   }

   return ret;

inval:
   ret->kind = mDECL_INVAL;
   return ret;
}

/* Unit */

static struct munit parse_unit(struct parser *self) {
   auto tok = advance(self);
   struct mdecl *fst = nullptr, *lst = fst;

   while (true) {
      switch (tok.kind) {
      case mTOK_INVAL:
         madeus("Parser found an invalid token.");

      case mTOK_EOF:
         goto end;

      case mTOK_DOC:
      case mTOK_EOL:
         advance(self);
         break;

      case mTOK_ID:
         /*
          * Declaration syntax is simple:
          * [ID][COLON][...] is object;
          * [ID][LPAREN][...] is function.
          */
         switch (self->snd.kind) {
         case mTOK_COLON:
            mferro(tok.loc, "Objects are not supported yet.");
            skipuntil(self, mTOK_EOL);
            break;

         case mTOK_LPAREN:
            auto fdecl = parse_func(self);
            if (!fst) {
               fst = fdecl;
               lst = fst;
            } else {
               lst->next = fdecl;
               lst = fdecl;
            }
            break;

         default:
            mferro(tok.loc, "Expected colon or left paren.");
            skipuntil(self, mTOK_EOL);
         }
         break;

      default:
         mferro(tok.loc, "Expected declaration.");
         skipuntil(self, mTOK_EOL);
      }

      tok = cur(self);
   }

end:
   return (struct munit){
      .decls = fst
   };
}

bool mparse_unit(const char *src) {
   /* The file exists? */
   FILE *file = fopen(src, "r");
   if (!file) {
      merro(
         "Unabe to load the '%s' unit, system: %s.",
         src,
         strerror(errno)
      );
      return false;
   }

   /* Gets the file size. */
   fseek(file, 0, SEEK_END);
   size_t filesz = ftell(file);

   /* Allocates a buffer and copies the file. */
   char *buf = malloc(filesz);
   fseek(file, 0, SEEK_SET);
   fread(buf, filesz, 1, file);
   buf[filesz] = '\0';

   /* Creates a instance. */
   struct mstrpool strpool = mstrpool_new();
   struct parser self = {
      .lex = mlexer_new(&strpool, src, buf, filesz),
      .buf = buf
   };

   /* Parse! */
   auto unit = parse_unit(&self);
   unit.name = src;

   /* Print! */
   munit_print(&unit);

   mstrpool_del(&strpool);
   fclose(file);
   free(buf);
   return true;
}
