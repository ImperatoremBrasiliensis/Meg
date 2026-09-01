/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Lexicon/Lexer.h>

#include <internal/Code/Lexicon/Token.h>
#include <internal/Orbita.h>
#include <internal/Output.h>

#include <ctype.h>
#include <string.h>

prosLexer prosLexer_new(
   prosVirtualMachine *vm,
   prosSource *src,
   prosIdTable *idTable
) {
   if (!prosSource_load(src, vm))
      pros_panic("prosLexer_new(): Unable to load source '$'.", src->filename);

   prosLexer ret = {
      .vm = vm,
      .src = src,
      .idTable = idTable,
      .indentationStack = prosVector_new(sizeof(uint32_t), nullptr),
      .tokArray = malloc(4 * sizeof(prosToken)),
      .arraySize = 4,
      .useTabAsIndentation = -1
   };

   return ret;
}

void prosLexer_del(prosLexer *self) {
   prosSource_unload(self->src, self->vm);
   prosVector_del(&self->indentationStack);
   free(self->tokArray);
   *self = (prosLexer){};
}

struct lexerLine {
   prosString beg;
   prosString end;
   uint32_t offset, size, ln, cl;
};

static inline prosTokenTok lexerIsKeyword(prosString str, size_t len) {
   struct Keyword {
      const char kw[16];
      prosTokenTok tk;
   };

   switch (len) {
   case 2: {
      constexpr struct Keyword len2[] = {
         {"if", PROS_TOK_KEYWORD_IF},
         {"or", PROS_TOK_KEYWORD_OR},
      };
      for (size_t i = 0; i < PROS_SIZEOF_ARRAY(len2); i++) {
         if (len2[i].kw[0] == str[0]) {
            if (!memcmp(len2[i].kw, str, len)) {
               return len2[i].tk;
            }
         }
      }
      break;
   }

   case 3: {
      constexpr struct Keyword len3[] = {
         {"val", PROS_TOK_KEYWORD_VAL},
         {"var", PROS_TOK_KEYWORD_VAR},
         {"int", PROS_TOK_KEYWORD_INT},
         {"for", PROS_TOK_KEYWORD_FOR}
      };
      for (size_t i = 0; i < PROS_SIZEOF_ARRAY(len3); i++) {
         if (len3[i].kw[0] == str[0]) {
            if (!memcmp(len3[i].kw, str, len)) {
               return len3[i].tk;
            }
         }
      }
      break;
   }

   case 4: {
      constexpr struct Keyword len4[] = {
         {"func", PROS_TOK_KEYWORD_FUNC},
         {"bool", PROS_TOK_KEYWORD_BOOL},
         {"char", PROS_TOK_KEYWORD_CHAR},
         {"true", PROS_TOK_KEYWORD_TRUE},
         {"else", PROS_TOK_KEYWORD_ELSE},
         {"loop", PROS_TOK_KEYWORD_LOOP}
      };
      for (size_t i = 0; i < PROS_SIZEOF_ARRAY(len4); i++) {
         if (len4[i].kw[0] == str[0]) {
            if (!memcmp(len4[i].kw, str, len)) {
               return len4[i].tk;
            }
         }
      }
      break;
   }

   case 5: {
      constexpr struct Keyword len5[] = {
         {"float", PROS_TOK_KEYWORD_FLOAT},
         {"false", PROS_TOK_KEYWORD_FALSE},
         {"while", PROS_TOK_KEYWORD_WHILE},
         {"break", PROS_TOK_KEYWORD_BREAK}
      };
      for (size_t i = 0; i < PROS_SIZEOF_ARRAY(len5); i++) {
         if (len5[i].kw[0] == str[0]) {
            if (!memcmp(len5[i].kw, str, len)) {
               return len5[i].tk;
            }
         }
      }
      break;
   }

   case 6: {
      constexpr struct Keyword len6[] = {
         {"repeat", PROS_TOK_KEYWORD_REPEAT},
         {"return", PROS_TOK_KEYWORD_RETURN}
      };
      for (size_t i = 0; i < PROS_SIZEOF_ARRAY(len6); i++) {
         if (len6[i].kw[0] == str[0]) {
            if (!memcmp(len6[i].kw, str, len)) {
               return len6[i].tk;
            }
         }
      }
      break;
   }

   case 8: {
      constexpr struct Keyword len7[] = {
         {"continue", PROS_TOK_KEYWORD_CONTINUE},
      };
      for (size_t i = 0; i < PROS_SIZEOF_ARRAY(len7); i++) {
         if (len7[i].kw[0] == str[0]) {
            if (!memcmp(len7[i].kw, str, len)) {
               return len7[i].tk;
            }
         }
      }
      break;
   }
   }

   return PROS_TOK_ID;
}

static inline prosToken lexerReadIdentifier(
   prosLexer *self,
   prosString str,
   size_t *cl
) {
   size_t len = 0;
   size_t maxLen = &self->src->content[self->src->size] - str;

   while (len < maxLen && (isalnum(str[len]) || str[len] == '_'))
      len++;

   prosTokenTok tok = lexerIsKeyword(str, len);
   prosToken ret = {
      .tok = tok,
      .data.id = tok == PROS_TOK_ID ?
         prosIdTable_pushId(self->idTable, str, len) :
         nullptr,
      .loc = {
         self->vm,
         self->src,
         len,
         self->offset,
         self->line,
         *cl + 1
      }
   };
   *cl += len;
   return ret;
}

static inline prosToken lexerReadNumberLiteral(
   prosLexer *self,
   prosString str,
   size_t *cl
) {
   size_t len = 0;
   size_t maxLen = &self->src->content[self->src->size] - str;

   while (len < maxLen && isalnum(str[len]))
      len++;

   prosToken ret = {
      .tok = PROS_TOK_LITERAL_NUMBER,
      .data.id = prosIdTable_pushId(self->idTable, str, len),
      .loc = {
         self->vm,
         self->src,
         len,
         self->offset,
         self->line,
         *cl + 1
      },
   };
   *cl += len;
   return ret;
}

static PROS_INLINE void lexerTokArrayAdd(prosLexer *self, prosToken *tok) {
   if (self->tokCount == self->arraySize) {
      self->arraySize *= 2;
      self->tokArray = realloc(
         self->tokArray,
         self->arraySize * sizeof(prosToken)
      );

      if (!self->tokArray) {
         pros_panic(
            "prosLexer: Call to `realloc()` resulted in error."
         );
      }
   }

   self->tokArray[self->tokCount] = *tok;
   self->tokCount++;
}

static bool lexerGetIndentation(prosLexer *self) {
begin:
   uint32_t ind = 0;
   prosString line = &self->src->content[self->offset];

   if (self->useTabAsIndentation == -1) {
      if (line[0] == ' ') {
         self->useTabAsIndentation = false;
      } else if (line[0] == '\t') {
         self->useTabAsIndentation = true;
      } else {
         // Line is not indented.
         return true;
      }
   }

   size_t maxInd = self->src->size - self->offset;
   if (self->useTabAsIndentation) {
      while (ind < maxInd) {
         if (line[ind] == '\t') {
            ind++;
            continue;
         }

         if (line[ind] == ' ') {
            goto indentError;
         }

         if (line[ind] == '\n') {
            self->offset += ind + 1;
            goto begin;
         }

         break;
      }
   } else {
      while (ind < maxInd) {
         if (line[ind] == ' ') {
            ind++;
            continue;
         }

         if (line[ind] == '\t') {
            goto indentError;
         }

         if (line[ind] == '\n') {
            self->offset += ind + 1;
            goto begin;
         }

         break;
      }
   }

   uint32_t currentInd = 0;
   if (self->indentationStack.size)
      currentInd = *(uint32_t *) prosVector_getLastObj(&self->indentationStack);

   if (ind == currentInd) {
      goto finalize;
   }

   else if (ind > currentInd) {
      prosVector_pushBack(&self->indentationStack, &ind);
      lexerTokArrayAdd(
         self,
         &(prosToken){
            .tok = PROS_TOK_INDENT,
            .data.identation = ind,
            .loc = {
               self->vm,
               self->src,
               ind,
               self->offset,
               self->line,
               1
            }
         }
      );

      goto finalize;
   }

   // In case of dedent.
   else {
      while (ind < currentInd) {
         prosVector_popBack(&self->indentationStack);
         lexerTokArrayAdd(
            self,
            &(prosToken){
               .tok = PROS_TOK_DEDENT,
               .data.identation = ind,
               .loc = {
                  self->vm,
                  self->src,
                  ind,
                  self->offset,
                  self->line,
                  1
               }
            }
         );

         if (self->indentationStack.size)
            currentInd = *(uint32_t *) prosVector_getLastObj(&self->indentationStack);
         else
            break;
      }

      // Dedent must not be followed by a indent.
      if (ind > currentInd) {
         prosOutput_reportError(
            (prosSourceLocation){
               self->vm,
               self->src,
               ind + 1,
               self->offset,
               self->line,
               1
            },
            "Broken identation.",
            nullptr
         );
         return false;
      }

      goto finalize;
   }

finalize:
   self->offset += ind;
   return true;

indentError:
   prosOutput_reportError(
      (prosSourceLocation){
         self->vm,
         self->src,
         ind,
         self->offset,
         self->line,
         1
      },
      "Using both tabs and spaces.",
      nullptr
   );
   return true;
}

bool prosLexer_getLine(prosLexer *self) {
   size_t cl = 0;

   self->tokp = 0;
   self->tokCount = 0;

   // Get line identation level (in spaces).
   if (!lexerGetIndentation(self))
      return false;

   prosTokenTok tok = PROS_TOK_EOF;
   prosString line = &self->src->content[self->offset];
   prosString eof = &self->src->content[self->src->size];

   while (&line[cl] < eof) {
      prosSourceLocation loc = {
         .vm = self->vm,
         .src = self->src,
         .offset = self->offset,
         .line = self->line + 1,
         .column = cl + 1
      };
      prosToken itok;
      size_t len = 1;  // Min token size.

      switch (line[cl]) {
      case '\n':
         lexerTokArrayAdd(
            self,
            &(prosToken){
               .tok = PROS_TOK_EOL,
               .loc = loc
            }
         );
         self->offset += cl + 1;
         self->line++;
         return true;

      // Skips spaces.
      case ' ':
      case '\t':
         while (&line[++cl] < eof && line[cl] == ' ') {
            continue;
         }
         continue;

         /* Identifiers */
         // clang-format off
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O':
			case 'P': case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X': case 'Y':
			case 'Z':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y': 
			case 'z':
			case '_':
         // clang-format on

         /*
          * If the identifier is a keyword, the keyword token
          * will be returned.
          */
         itok = lexerReadIdentifier(self, &line[cl], &cl);
         lexerTokArrayAdd(self, &itok);
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
         itok = lexerReadNumberLiteral(self, &line[cl], &cl);
         lexerTokArrayAdd(self, &itok);
         continue;

      case '(':
         tok = PROS_TOK_SCOPE_PAREN_OPEN;
         break;
      case ')':
         tok = PROS_TOK_SCOPE_PAREN_CLOSE;
         break;
      case ':':
         tok = PROS_TOK_PUNCT_COLON;
         break;
      case ',':
         tok = PROS_TOK_PUNCT_COMMA;
         break;

      case '+':
         if (&line[cl + 1] < eof) {
            if (line[cl + 1] == '=') {
               tok = PROS_TOK_OPT_PLUSEQUAL;
               len = 2;
               break;
            }
         }

         tok = PROS_TOK_OPT_PLUS;
         break;

      case '-':
         if (&line[cl + 1] < eof) {
            if (line[cl + 1] == '=') {
               tok = PROS_TOK_OPT_MINUSEQUAL;
               len = 2;
               break;
            }
         }

         tok = PROS_TOK_OPT_MINUS;
         break;

      case '*':
         if (&line[cl + 1] < eof) {
            if (line[cl + 1] == '=') {
               tok = PROS_TOK_OPT_STAREQUAL;
               len = 2;
               break;
            }
         }

         tok = PROS_TOK_OPT_STAR;
         break;
      case '/':
         if (&line[cl + 1] < eof) {
            if (line[cl + 1] == '=') {
               tok = PROS_TOK_OPT_SLASHEQUAL;
               len = 2;
               break;
            }
            if (line[cl + 1] == '/') {
               while (&line[cl] < eof && line[cl] != '\n') {
                  cl++;
               }
               continue;
            }
         }

         tok = PROS_TOK_OPT_SLASH;
         break;
      case '=':
         tok = PROS_TOK_OPT_EQUAL;
         break;
      case '!':
         tok = PROS_TOK_OPT_EXCLAMATION;
         break;

      default:
         prosOutput_reportError(
            loc,
            "Unknown token.",
            nullptr
         );
         return false;
      }

      lexerTokArrayAdd(
         self,
         &(prosToken){
            .tok = tok,
            .loc = loc
         }
      );
      cl += len;
   }

   lexerTokArrayAdd(
      self,
      &(prosToken){
         .tok = PROS_TOK_EOF,
         .loc = {
            self->vm,
            self->src,
            1,
            self->offset,
            self->line,
            cl + 2
         }
      }
   );
   self->offset += cl;
   return false;
}
