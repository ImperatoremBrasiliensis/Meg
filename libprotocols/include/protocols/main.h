/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_MAIN_H
#define PROTOCOLS_MAIN_H

#include <protocols/Event.h>
#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

#include <stdint.h>

typedef struct prosOrbita *prosOrbita;

struct prosOfficer_Control {
	// Event receiver.
	bool (*receiver)(prosOrbita orbita, prosEvent event);
	// Commanders.
	bool (*mapProtocols)(prosOrbita orbita);
	bool (*gotoProtocol)(prosOrbita orbita);
	bool (*getNextStatement)(prosOrbita);
	bool (*getPreviousStatement)(prosOrbita orbita);
};

typedef struct prosOfficer *prosOfficer;

prosOfficer prosOffice_new(prosString name, struct prosOfficer_Control *p);

prosOfficer prosOfficer_del(prosOfficer self, prosString name);

[[nodiscard]]
prosOrbita prosOrbita_new(prosString name, prosOfficer officer);

[[nodiscard]]
prosOrbita prosOrbita_del(prosOrbita self);

PROTOCOLS_EXTERNC_END

#endif	  // PROTOCOLS_MAIN_H
