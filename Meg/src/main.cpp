/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <Meg/debug.hpp>
#include <Meg/filesystem.hpp>

#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <unistd.h>

#include "compiler/compiler.hpp"

using namespace Meg;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		dbg::prt_def("You must specify a sub-command before we go on.\n");
		return 1;
	}

	if (!strcmp(argv[1], "compilare"))
		return compiler::compile(argc - 2, argv + 2);

	dbg::prt_def("The subcommand '%s' does not exists.\n", argv[1]);
	return 1;
}