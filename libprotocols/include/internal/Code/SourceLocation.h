/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_SOURCELOCATION_H
#define PROTOCOLS_INTERNAL_CODE_SOURCELOCATION_H

#include <internal/Code/Source.h>
#include <internal/Utilities.h>
#include <internal/VirtualMachine.h>

typedef struct prosSourceLocation_s {
	prosVirtualMachine *vm;
	prosSource *src;
	uint32_t len, offset, line, column;
} prosSourceLocation;

#endif	// PROTOCOLS_INTERNAL_CODE_SOURCELOCATION_H
