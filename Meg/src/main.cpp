#include "compiler/compiler.hpp"

/* STD */
#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "You must specify a sub-command before we go on." << std::endl;
		return 1;
	}

	if (!strcmp(argv[1], "compilare"))
		return compiler::compile(argc - 2, argv + 2);

	std::cout << "The sub-command '" << argv[1] << "' does not exists." << std::endl;

	return 1;
}