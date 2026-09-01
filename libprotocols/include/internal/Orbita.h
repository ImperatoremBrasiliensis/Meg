/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_ORBITA_H
#define PROTOCOLS_INTERNAL_ORBITA_H

#include <protocols/Orbita.h>

#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

struct prosOrbita_s {
	prosString name;
	int vmCount;
	void *vm;
};

void pros_error(prosOrbita orbita, prosString name, prosString msg);

void pros_fatal(prosOrbita orbita, prosString name, prosString msg);

[[noreturn]]
void pros_panic(prosString msg, ...);

PROTOCOLS_EXTERNC_END

#endif	  // PROTOCOLS_INTERNAL_ORBITA_H
