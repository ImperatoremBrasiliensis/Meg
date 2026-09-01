/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_EVENT_H
#define PROTOCOLS_EVENT_H

#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

#include <stdint.h>

typedef enum prosStandardEventId {
	PROS_SUCCESS,
	PROS_ERROR,
	PROS_FATAL_ERROR,
	PROS_SYSTEM_ERROR,
	PROS_PANIC
} prosStandardEventId;

typedef struct prosEvent {
	prosStandardEventId number;
	prosString id;
	prosString message;
} prosEvent;

PROTOCOLS_EXTERNC_END

#endif	  // PROTOCOLS_EVENT_H
