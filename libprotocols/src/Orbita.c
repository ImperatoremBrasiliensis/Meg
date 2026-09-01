/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Orbita.h>
#include <protocols/Orbita.h>

#include <internal/Utilities.h>

#include <stdarg.h>
#include <stdlib.h>

void pros_error(prosOrbita orbita [[maybe_unused]], prosString name [[maybe_unused]], prosString msg [[maybe_unused]]) {
	abort();
}

void pros_fatal(prosOrbita orbita [[maybe_unused]], prosString name [[maybe_unused]], prosString msg) {
	pros_print(STDERR_FILENO, "\033[1;31mFatal Error\033[0;31m, not handled\033[0m:", msg);
	abort();
}

[[noreturn]]
void pros_panic(prosString msg, ...) {
	va_list va;
	va_start(va);
	pros_print__va(STDERR_FILENO, "\033[1;31mPanic\033[0m", msg, va);
	va_end(va);
	abort();
}

prosOrbita prosOrbita_new(prosString name) {
	if (!name)
		pros_panic("`name` parameter must be soecified.");

	prosOrbita ret = malloc(sizeof(struct prosOrbita_s));
	*ret = (struct prosOrbita_s){
		.name = name,
		.vm = nullptr
	};
	return ret;
}

prosOrbita prosOrbita_del(prosOrbita *self) {
	free(*self);
	return nullptr;
}
