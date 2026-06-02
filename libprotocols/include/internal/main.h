/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_MAIN_H
#define PROTOCOLS_INTERNAL_MAIN_H

#include <protocols/main.h>

#include <internal/code/Bytecode.h>
#include <internal/code/Source.h>

struct prosOfficer {
	struct prosOfficer_Control ctrl;
	prosString id;
	prosEvent eventStack[];
};

bool prosOfficer_receive(prosOfficer officer, prosOrbita orbita, prosEvent event);

struct prosOrbita {
	prosString name;
	prosOfficer officer;
	prosSourceSet sources;
	prosBytecodeSet bytecodes;
};

void pros_error(prosOrbita orbita, prosString name, prosString msg);
void pros_error(prosOrbita orbita, prosString name, prosString msg);
void pros_panic(prosString msg);

#endif	  // PROTOCOLS_INTERNAL_MAIN_H
