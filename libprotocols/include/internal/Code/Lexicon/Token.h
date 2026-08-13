/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_LEXICON_TOKEN_H
#define PROTOCOLS_INTERNAL_LEXICON_TOKEN_H

#include <internal/Code/Lexicon/IdTable.h>
#include <internal/Code/Source.h>
#include <internal/Code/SourceLocation.h>
#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef enum prosTokenTok_e : uint8_t {
   /* */
   PROS_TOK_NONE,
   /* Indentation scope bounder */
   PROS_TOK_INDENT,
   PROS_TOK_DEDENT,
   /* Literals */
   PROS_TOK_LITERAL_NUMBER,
   /* Keywords */
   /// Declarations.
   PROS_TOK_KEYWORD_FUNC,
   PROS_TOK_KEYWORD_VAR,
   PROS_TOK_KEYWORD_VAL,
   /// Statements.
   PROS_TOK_KEYWORD_IF,
   PROS_TOK_KEYWORD_OR,
   PROS_TOK_KEYWORD_ELSE,
   PROS_TOK_KEYWORD_WHILE,
   PROS_TOK_KEYWORD_FOR,
   PROS_TOK_KEYWORD_REPEAT,
   PROS_TOK_KEYWORD_LOOP,
   PROS_TOK_KEYWORD_BREAK,
   PROS_TOK_KEYWORD_CONTINUE,
   PROS_TOK_KEYWORD_RETURN,
   /// Built-in types.
   PROS_TOK_KEYWORD_INT,
   PROS_TOK_KEYWORD_BOOL,
   PROS_TOK_KEYWORD_FLOAT,
   PROS_TOK_KEYWORD_CHAR,
   /// Built-in literals.
   PROS_TOK_KEYWORD_TRUE,
   PROS_TOK_KEYWORD_FALSE,
   /* Punctuation */
   PROS_TOK_PUNCT_COLON,
   PROS_TOK_PUNCT_COMMA,
   /* Operators */
   PROS_TOK_OPT_EQUAL,
   PROS_TOK_OPT_PLUSEQUAL,
   PROS_TOK_OPT_MINUSEQUAL,
   PROS_TOK_OPT_STAREQUAL,
   PROS_TOK_OPT_SLASHEQUAL,
   PROS_TOK_OPT_PLUS,
   PROS_TOK_OPT_MINUS,
   PROS_TOK_OPT_STAR,
   PROS_TOK_OPT_SLASH,
   PROS_TOK_OPT_EXCLAMATION,
   /* Scope bounders */
   PROS_TOK_SCOPE_PAREN_OPEN,
   PROS_TOK_SCOPE_PAREN_CLOSE,
   /* Identifier */
   PROS_TOK_ID,
   /* End of line and file */
   PROS_TOK_EOL,
   PROS_TOK_EOF
} prosTokenTok;

typedef struct prosToken_s {
   prosSourceLocation loc;
   prosTokenTok tok;
   union {
      prosId *id;
      size_t identation;
   } data;
} prosToken;

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_LEX_TOKEN_H
