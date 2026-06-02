/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_BYTECODE_H
#define PROTOCOLS_INTERNAL_CODE_BYTECODE_H

#include <internal/code/Source.h>

#include <stdint.h>
#include <stdlib.h>

typedef struct prosBytecode {
	size_t size;
	prosSource src;
	char bytecode[];
} *prosBytecode;

typedef struct prosBytecodeSet {
	int quantity;
	int last;
	prosBytecode bytecode[];
} *prosBytecodeSet;

#endif	  // PROTOCOLS_INTERNAL_CODE_BYTECODE_H
