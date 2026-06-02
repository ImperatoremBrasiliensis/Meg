/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <Meg/lang.hpp>

#include <Meg/debug.hpp>

#include <cerrno>
#include <cstdio>
#include <cstring>

using namespace Meg;

struct {
	FILE *lat;
	struct {
		FILE *br;
	} por;
} languages;

bool lang::init(const char *lang_dir_path [[maybe_unused]]) noexcept {
	languages.lat = fopen("lat.lang", "rb");
	if (!languages.lat)
		dbg::prt_ferr(
			"Fasciculus translationis principalis ('lat.lang') non inventus est: %s.\n",
			strerror(errno)
		);
	return true;
}