/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_SOURCE_H
#define PROTOCOLS_INTERNAL_CODE_SOURCE_H

#include <internal/Code/Bytecode.h>
#include <internal/Utilities.h>
#include <protocols/VirtualMachine.h>

PROTOCOLS_EXTERNC_START

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct prosSource_s {
	prosVirtualMachine *vm;

	prosString filename;
	char *filePath;
	prosBytecode bytecode;
	prosString content;
	size_t size;
	/* Code register. */
	prosVector functionTable;
	prosVector globalVariableTable;
	prosVector definedTypeTable;
	prosArena data;

	int refCount;
} prosSource;

[[nodiscard]]
prosSource prosSource_new(prosString filename, prosVirtualMachine *vm, bool load);

void prosSource_del(prosSource *self);

bool prosSource_load(prosSource *self, prosVirtualMachine *vm);

void prosSource_unload(prosSource *self, prosVirtualMachine *vm);

PROTOCOLS_EXTERNC_END

#endif	// PROTOCOLS_INTERNAL_CODE_SOURCE_H
