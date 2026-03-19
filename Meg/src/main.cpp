#include <cstring>

#include <debug.hpp>

#include "compiler/compiler.hpp"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		dbg::print(DEFT, "You must specify a sub-command before we go on.\n");
		return 1;
	}

	if (!strcmp(argv[1], "compilare"))
		return compiler::compile(argc - 2, argv + 2);

	dbg::print(DEFT, "The subcommand '%s' does not exists.\n", argv[1]);
	return 1;
}