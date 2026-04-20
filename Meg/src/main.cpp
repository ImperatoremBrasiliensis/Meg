#include <lunique/debug.hpp>
#include <lunique/filesystem.hpp>

#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <unistd.h>

#include "compiler/compiler.hpp"

int main(int argc, char *argv[]) {
	lunique::dbg::init(true);

	if (argc < 2) {
		lunique::dbg::prt_def("You must specify a sub-command before we go on.\n");
		return 1;
	}

	if (!strcmp(argv[1], "compilare"))
		return compiler::compile(argc - 2, argv + 2);

	lunique::dbg::prt_def("The subcommand '%s' does not exists.\n", argv[1]);
	return 1;
}