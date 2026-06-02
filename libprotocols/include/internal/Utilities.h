/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_UTILITIES_H
#define PROTOCOLS_INTERNAL_UTILITIES_H

#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

#ifdef _WIN32
#else
#	include <unistd.h>
#endif

#include <threads.h>

bool prosSyscall_write(int fd, prosString str, size_t size);

PROTOCOLS_EXTERNC_END

#endif	  // PROTOCOLS_INTERNAL_UTILITIES_H
