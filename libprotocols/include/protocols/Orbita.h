/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_ORBITA_H
#define PROTOCOLS_ORBITA_H

#include <protocols/Event.h>
#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

typedef struct prosOrbita_s *prosOrbita;

prosOrbita prosOrbita_new(prosString name);

prosOrbita prosOrbita_del(prosOrbita *self);

PROTOCOLS_EXTERNC_END

#endif	  // PROTOCOLS_ORBITA_H
