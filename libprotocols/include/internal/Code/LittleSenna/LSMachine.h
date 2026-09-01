/*
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_LITTLESENNA_LSMACHINE_H
#define PROTOCOLS_INTERNAL_CODE_LITTLESENNA_LSMACHINE_H

#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

#include <internal/Code/LittleSenna/Bytecode.h>

typedef struct prosLSMachine_s {
   uint64_t regs[16], sregs[16];
   char *stack;
   size_t stackSize;
   prosAllocator altor;
} prosLSMachine;

prosLSMachine prosLSMachine_new(size_t stackSize);

void prosLSMachine_del(prosLSMachine *self);

bool prosLSMachine_exec(prosLSMachine *self, prosInstruction intrs[]);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_CODE_LITTLESENNA_LSMACHINE_H
