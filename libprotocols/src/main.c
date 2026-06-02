/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <protocols/main.h>

#include <internal/Utilities.h>
#include <internal/main.h>

#include <stdlib.h>

void pros_error(prosOrbita orbita, prosString name, prosString msg) {
	prosEvent event = {PROS_ERROR, name, msg};
	orbita->officer->ctrl.receiver(orbita, event);
}

void pros_fatal(prosOrbita orbita, prosString name, prosString msg) {
	prosEvent event = {PROS_FATAL_ERROR, name, msg};
	orbita->officer->ctrl.receiver(orbita, event);
	pros_print(STDERR_FILENO, "\033[1;31mFatal Error\033[0;31m, not handled\033[0m:", msg);
	abort();
}

[[noreturn]]
void pros_panic(prosString msg) {
	pros_print(STDERR_FILENO, "\033[1;31mPanic\033[0m:", msg);
	abort();
}

prosOrbita prosOrbita_new(prosString name, prosOfficer officer) {
	if (!officer)
		pros_panic("Valid officer don't specified.");

	prosOrbita ret = malloc(sizeof(struct prosOrbita));
	*ret = (struct prosOrbita){
		.name = name,
		.officer = officer
	};
	return ret;
}

prosOrbita prosOrbita_del(prosOrbita self) {
	free(self);
	return nullptr;
}
