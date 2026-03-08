#include "compiler/compiler.hpp"

/* STD */
#include <cstring>

int main(int argc, char* argv[]) {
	if (argc < 2)
		return 1;
	if (!strcmp(argv[1], "compile"))
		return compiler::compile(--argc, ++argv);
	return 2;
}