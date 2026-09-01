/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_BYTECODE_H
#define PROTOCOLS_INTERNAL_CODE_BYTECODE_H

#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

#include <stdint.h>
#include <stdlib.h>

typedef struct prosBytecode_s {
	size_t size;
	prosVector code;
} *prosBytecode;

[[nodiscard]]
prosBytecode prosBytecode_new();

void prosBytecode_del(prosBytecode *self);

PROTOCOLS_EXTERNC_END

#endif	  // PROTOCOLS_INTERNAL_CODE_BYTECODE_H
