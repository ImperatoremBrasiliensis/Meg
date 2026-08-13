/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_PARSE_PARSER_H
#define PROTOCOLS_INTERNAL_CODE_PARSE_PARSER_H

#include <internal/Code/Lexicon/Lexer.h>
#include <internal/Code/Semantic/Semantics.h>
#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef struct prosParser_s {
	prosLexer lex;
	prosSource *src;
	prosVirtualMachine *vm;
	prosSemantics sem;
} prosParser;

prosParser prosParser_new(
	prosSource *src,
	prosVirtualMachine *vm,
	prosIdTable *idTable
);

void prosParser_del(prosParser *self);

bool prosParser_parseAll(prosParser *self);

PROTOCOLS_EXTERNC_END

#endif	// PROTOCOLS_INTERNAL_CODE_PARSE_PARSER_H
