/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_LEXICON_LEXER_H
#define PROTOCOLS_INTERNAL_LEXICON_LEXER_H

#include <internal/Code/Semantic/Scope.h>
#include <internal/Code/Source.h>
#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef struct prosLexerIdentifier_s {
   size_t id;
   size_t len;
} prosLexerIdentifier;

typedef struct prosLexer_s {
   prosVirtualMachine *vm;
   prosSource *src;
   prosIdTable *idTable;
   prosVector indentationStack;
   prosToken *tokArray;
   size_t arraySize, tokCount;
   size_t tokp /* Parser only */;
   size_t offset, line;
   int useTabAsIndentation;
} prosLexer;

prosLexer prosLexer_new(
   prosVirtualMachine *vm,
   prosSource *src,
   prosIdTable *idTable
);

void prosLexer_del(prosLexer *self);

bool prosLexer_getLine(prosLexer *self);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_LEX_LEXER_H
