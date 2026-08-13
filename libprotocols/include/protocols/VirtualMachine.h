/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_VIRTUAL_MACHINE_H
#define PROTOCOLS_VIRTUAL_MACHINE_H

#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef enum prosVirtualMachineType_e : uint8_t {
	PROS_VIRTUAL_MACHINE_DECLARATIVE,
	PROS_VIRTUAL_MACHINE_IMPERATIVE,
	PROS_VIRTUAL_MACHINE_LIBRARY
} prosVirtualMachineType;

typedef struct prosVirtualMachine_Config_s {
	prosString name;
	prosString workingDirectory;
	prosString mainFile;
	prosVirtualMachineType vmType;
} prosVirtualMachine_Config;

typedef struct prosVirtualMachine_s *prosVirtualMachine;

[[nodiscard]]
prosVirtualMachine prosVirtualMachine_new(prosVirtualMachine_Config pros);

void prosVirtualMachine_del(prosVirtualMachine *self);

int prosVirtualMachine_run(prosVirtualMachine *self, bool ownProcess);

PROTOCOLS_EXTERNC_END

#endif	// PROTOCOLS_VIRTUAL_MACHINE_H
