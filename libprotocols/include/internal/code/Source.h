/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_SOURCE_H
#define PROTOCOLS_INTERNAL_CODE_SOURCE_H

#include <protocols/main.h>

#include <stdint.h>
#include <stdlib.h>

#include <sys/stat.h>

#ifdef _WIN32
#	define stat(name, structStat) _stat(name, structStat)
#endif

typedef struct prosSource {
	prosString name;
	size_t size;
} *prosSource;

prosSource prosSource_new__I(prosString filename);

typedef struct prosSourceSet {
	uint16_t quantity;
	uint16_t last;
	prosSource data[];
} *prosSourceSet;

#endif	  // PROTOCOLS_INTERNAL_CODE_SOURCE_H
