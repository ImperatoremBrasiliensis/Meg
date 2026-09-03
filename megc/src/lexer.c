/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <megc/diagno.h>
#include <megc/lexer.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct mlexer mlexer_new(
   struct mstrpool *strpool,
   const char *fname,
   const char *buf,
   unsigned n
) {
   return (struct mlexer){
      .strpool = strpool,
      .fname = fname,
      .buf = buf,
      .busz = n,
      .line = 1,
      .column = 1
   };
}

static inline char getch(struct mlexer *self) {
   if (self->off < self->busz) {
      char c = self->buf[self->off++];
      if (c == '\n') {
         self->line++;
         self->column = 0;
      }

      self->column++;
      return c;
   }

   return '\0';
}

static inline void ungetch(struct mlexer *self) {
   if (self->column > 1) {
      self->column--;
      self->off--;
   }
}

static inline void readuntil(struct mlexer *self, char c) {
   char ch = getch(self);
   while (ch != c) {
      ch = getch(self);
      if (ch == '\0') {
         break;
      }
   }
}

static enum mtoken_kind iskeyword(
   const char *id,
   size_t len
) {
   constexpr struct {
      size_t len;
      const char kw[16];
      enum mtoken_kind kind;
   } keywords[] = {
      {4, "type", mTOK_TYPE},
      {3, "let", mTOK_LET},
      {3, "mut", mTOK_MUT},
      {3, "new", mTOK_NEW},
      {3, "del", mTOK_DEL},
      {5, "defer", mTOK_DEFER},
      {2, "if", mTOK_IF},
      {2, "or", mTOK_OR},
      {4, "else", mTOK_ELSE},
      {5, "while", mTOK_WHILE},
      {3, "for", mTOK_FOR},
      {4, "loop", mTOK_LOOP},
      {5, "break", mTOK_BREAK},
      {8, "continue", mTOK_CONTINUE}
   };

   constexpr int arrsz =
      sizeof keywords / sizeof keywords[0];

   for (int i = 0; i < arrsz; i++) {
      if (keywords[i].len == len) {
         if (!memcmp(keywords[i].kw, id, len)) {
            return keywords[i].kind;
         }
      }
   }

   return mTOK_ID;
}

static struct mtoken getid(
   struct mlexer *self,
   struct mloc loc
) {
   const char *beg = &self->buf[self->off];
   size_t len = 0;

   while (self->off < self->busz) {
      char c = self->buf[self->off];
      if (!isalnum(c) && c != '_') {
         break;
      }

      len++;
      self->off++;
   }

   const char *entry = mstrpool_insert(
      self->strpool,
      beg,
      len
   );

   self->column += len;
   return (struct mtoken){
      .kind = iskeyword(beg, len),
      .loc = loc,
      .lit = entry
   };
}

static char getscape(struct mlexer *self) {
   struct mloc loc = {
      .filename = self->fname,
      .line = self->line,
      .column = self->column
   };

   char c = getch(self);
   switch (c) {
   case '\0':
      mferro(loc, "Unterminated scape sequence.");
      return 0;
   case '0':
      return '\0';
   case 'n':
      return '\n';
   case 't':
      return '\t';
   case 'a':
      return '\a';
   case 'b':
      return '\b';
   case 'r':
      return '\r';
   case 'v':
      return '\v';
   case 'f':
      return '\f';
   case '\\':
      return '\\';
   case '"':
      return '"';
   case '\'':
      return '\'';
   case '?':
      return '\?';

   default:
      mferro(loc, "Unknown scape sequence: '%c'.", c);
      ungetch(self);
      return 0;
   }
}

static const char *getstr(struct mlexer *self) {
   struct mloc loc = {
      .filename = self->fname,
      .line = self->line,
      .column = self->column
   };

   char str[16 * 1024];
   size_t len = 0;

   char c;
   while (true) {
      c = getch(self);
      if (c == '"') {
         break;
      }

      if (len >= sizeof str) {
         mferro(loc, "String too large.");
         while (c != '"') {
            c = getch(self);
            if (c == '\0' || c == '\n') {
               auto cloc = loc;
               cloc.column = self->column;
               mferro(cloc, "Unterminated string.");
               readuntil(self, '\'');  // Tries to find '"'.
               return nullptr;
            }
         }
         break;
      }
      switch (c) {
      case '\n':
      case '\0':
         auto cloc = loc;
         cloc.column = self->column;
         mferro(cloc, "Unterminated string literal.");
         readuntil(self, '\'');  // Tries to find '"'.
         return nullptr;

      case '\\':
         c = getscape(self);
         str[len++] = c;
         break;

      default:
         str[len++] = c;
      }
   }

   // Inserts the string in the pool.
   return mstrpool_insert(
      self->strpool,
      str,
      len
   );
}

static struct mtoken getnum(struct mlexer *self, char c) {
   struct mtoken ret = {
      .loc = {
         .filename = self->fname,
         .line = self->line,
         .column = self->column - 1
      },
      .kind = mTOK_INTEGER
   };

   int base = 10;
   bool f = false;  // Is float.
   const char *bname = "decimal";

   /* Checks the numeber prefix. */
   if (c == '0') {
      char p = getch(self);
      switch (p) {
      case 'o':
         base = 8;
         bname = "octal";
         break;
      case 'b':
         base = 2;
         bname = "binary";
         break;
      case 'x':
         base = 16;
         bname = "hexadecimal";
         break;
      default:
         ungetch(self);
      }
      c = getch(self);
   }

   char buf[1024];
   size_t busz = 0;
   bool issep = false;
   int64_t val = 0;
   while (true) {
      auto cloc = ret.loc;
      cloc.column++;

      switch (c) {
      case '\0':
         goto final;

      case '.':
         if (f) {
            mferro(cloc, "Extra '.'.");
         } else {
            buf[busz++] = '.';
         }
         f = true;
         /* throughout */
      case ';':
         if (issep) {
            mferro(cloc, "Consecutive separators.");
         }
         issep = true;
         c = getch(self);
         continue;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         val = c - '0';
         goto eval;
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
         if (base != 16) {
            mferro(cloc, "Using hexadecimal digits in a %s literal.", bname);
            break;
         }
         val = (c - 'a') + 10;
         /* throughout */
eval:
         if (val >= base) {
            mferro(cloc, "Invalid digit in a %s literal.", bname);
            break;
         }

         buf[busz++] = c;
         break;

      case 'E':
         buf[busz++] = 'e';
         f = true;
         c = getch(self);

         /* Appends the signal. */
         if (c == '-' || c == '+') {
            buf[busz++] = c;
            c = getch(self);
         }

         while (isdigit(c)) {
            buf[busz++] = c;
            c = getch(self);
         }
         /* Goes one char back. */
         ungetch(self);

         /* Checks out if the exponent has digits. */
         c = self->buf[self->off - 1];
         if (c == 'E' || c == '-' || c == '+') {
            mferro(cloc, "Exponent has no digits.");
         }
         goto final;
      default:
         ungetch(self);
         goto final;
      }

      if (busz == sizeof buf) {
         mferro(ret.loc, "Number too long.");
         /* kip loop 'gambiarra'. */
         while (
            isdigit(c) ||
            (base == 16 ?
                  c == 'a' ||
                     c == 'b' ||
                     c == 'c' ||
                     c == 'd' ||
                     c == 'e' ||
                     c == 'f' :
                  false)
         ) {
            if (c == 'E') {
               c = getch(self);
               while (
                  isdigit(c) ||
                  c == '+' ||
                  c == '-'
               ) {
                  c = getch(self);
               }
            }

            c = getch(self);
         }

         return (struct mtoken){
            .kind = mTOK_INVAL
         };
      }
      c = getch(self);
      issep = false;
   }

final:
   /* Trying to not return nullptr. */
   if (busz == 0) {
      buf[0] = 0;
      busz = 1;
   }

   /* Adds in the sting pool. */
   ret.lit = mstrpool_insert(
      self->strpool,
      buf,
      busz
   );
   ret.kind = f ? mTOK_FLOAT : mTOK_INTEGER;
   return ret;
}

struct mtoken mlexer_lex(struct mlexer *self) {
again:
   struct mtoken ret = {
      .loc = {
         .filename = self->fname,
         .line = self->line,
         .column = self->column
      },
      .kind = mTOK_INVAL
   };

   char c = getch(self);
   if (isspace(c)) {
      while (isspace(c)) {
         if (c == '\n') {
            ret.kind = mTOK_EOL;
            goto end;
         }

         if (self->off < self->busz) {
            c = getch(self);
            continue;
         }
         goto end;
      }

      ungetch(self);
      goto again;
   }

   if (isalpha(c) || c == '_') {
      ungetch(self);
      return getid(self, ret.loc);
   }

   if (isdigit(c)) {
      return getnum(self, c);
   }

   switch (c) {
      char tc = 0;

   case '\0':
      ret.kind = mTOK_EOF;
      break;

   case '\\':
      readuntil(self, '\\');
      ret.kind = mTOK_DOC;
      break;

   case '"':
      ret.kind = mTOK_STRING;
      ret.lit = getstr(self);
      break;
   case '\'':
      ret.kind = mTOK_CHAR;
      char ch;

      /* Gets the content. */
      ch = getch(self);
      if (ch == '\\') {  // Is scape.
         ch = getscape(self);
      } else {
         if (ch == '\'') {  // Is the left quote.
            mferro(ret.loc, "Empty char literal.");
            ret.lit = " ";
            break;
         } else if (ch == '\0' || ch == '\n') {  // Is EOF or EOL.
            mferro(ret.loc, "Unterminated char literal.");
            readuntil(self, '\'');  // Tries to find '\''.
            ret.lit = " ";
            break;
         }
      }

      c = getch(self);
      if (c != '\'') {
         mferro(ret.loc, "Incomplete char literal.");
         readuntil(self, '\'');
         ret.lit = " ";
         break;
      }

      ret.lit = mstrpool_insert(
         self->strpool,
         &ch,
         1
      );
      break;

   case '+':
      ret.kind = mTOK_ADD;
      break;
   case '-':
      ret.kind = mTOK_SUB;
      break;
   case '*':
      ret.kind = mTOK_MUL;
      break;
   case '/':
      ret.kind = mTOK_DIV;
      break;
   case '%':
      ret.kind = mTOK_MOD;
      break;
   case '&':
      tc = getch(self);
      if (tc == '&') {
         ret.kind = mTOK_LAND;
         break;
      }
      if (tc != '\0') {
         ungetch(self);
      }
      ret.kind = mTOK_AND;
      break;
   case '|':
      tc = getch(self);
      if (tc == '|') {
         ret.kind = mTOK_LOR;
         break;
      }
      if (tc != '\0') {
         ungetch(self);
      }
      ret.kind = mTOK_BOR;
      break;
   case '^':
      ret.kind = mTOK_EOR;
      break;
   case '!':
      ch = getch(self);
      if (ch == '=') {
         ret.kind = mTOK_NEQ;
         break;
      }
      if (ch != '\0') {
         ungetch(self);
      }
      ret.kind = mTOK_NEG;
      break;

   case ',':
      ret.kind = mTOK_COMMA;
      break;
   case ':':
      ret.kind = mTOK_COLON;
      break;
   case ';':
      ret.kind = mTOK_SEMIC;
      break;
   case '(':
      ret.kind = mTOK_LPAREN;
      break;
   case '[':
      ret.kind = mTOK_LBRCKT;
      break;
   case '{':
      ret.kind = mTOK_LBRACE;
      break;
   case ')':
      ret.kind = mTOK_RPAREN;
      break;
   case ']':
      ret.kind = mTOK_RBRCKT;
      break;
   case '}':
      ret.kind = mTOK_RBRACE;
      break;
   case '=':
      ch = getch(self);
      if (ch == '=') {
         ret.kind = mTOK_EQL;
         break;
      }
      if (ch != '\0') {
         ungetch(self);
      }
      ret.kind = mTOK_ASSIGN;
      break;
   case '>':
      ch = getch(self);
      if (ch == '=') {
         ret.kind = mTOK_GEQ;
         break;
      }
      if (ch != '\0') {
         ungetch(self);
      }
      ret.kind = mTOK_GTR;
      break;
   case '<':
      ch = getch(self);
      if (ch == '=') {
         ret.kind = mTOK_LEQ;
         break;
      }
      if (ch == '\0') {
         ungetch(self);
      }
      ret.kind = mTOK_LSS;
      break;

   case '$':
      ret.kind = mTOK_DOLLAR;
      break;
   case '~':
      ret.kind = mTOK_TILDE;
      break;

   default:
      merro("Undefined character '%c'.", c);
   }

end:
   return ret;
}
