/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_VIRTUAL_MACHINE_H
#define PROTOCOLS_INTERNAL_VIRTUAL_MACHINE_H

#include <protocols/VirtualMachine.h>

#include <internal/Code/Source.h>

PROTOCOLS_EXTERNC_START

struct prosVirtualMachine_s {
	prosSource mainSource;
	prosString name;
	prosString cwd;
	prosVector sources;
	int (*pushError)(prosVirtualMachine *self, prosString msg, ...);
	prosVirtualMachineType vmType;
};

PROTOCOLS_EXTERNC_END

#endif	// PROTOCOLS_INTERNAL_VIRTUAL_MACHINE_H
