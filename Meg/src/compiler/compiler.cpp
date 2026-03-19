#include "compiler.hpp"

#include <debug.hpp>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <vector>

struct {
	const char* output;
	bool shared;
} configs;

std::vector<const char*> compiler::sources;

// static bool config(int argc, char* argv[]);

static bool process_args(int argc, char* argv[]) {
	if (argc < 1)
		goto nosources;
	for (int i = 0; i < argc; i++) {
		if (argv[i][0] == '-') {
			// Flags
			if (!strcmp(argv[i], "-shared")) {
				configs.shared = true;
				continue;
			}

			// Options
			if (!strcmp(argv[i], "-o") && (i + 1) < argc) {
				configs.output = argv[i + 1];
				continue;
			} else {
				dbg::print(ERRO, "Unknown option '%s'.\n", argv[i]);
				return false;
			}

			printf("Unknown option or flag '%s'.\n", argv[i]);
			return false;
		}
		// Sources
		if (std::filesystem::exists(argv[i]))
			compiler::sources.push_back(argv[i]);
		else {
			std::cout << "The file '" << argv[i] << "' does nit exists." << std::endl;
			return false;
		}
	}

	if (compiler::sources.empty())
		goto nosources;

	return true;

nosources:
	std::cout << "No sources given." << std::endl;
	return false;
}

int compiler::compile(int argc, char* argv[]) noexcept {
	if (!process_args(argc, argv))
		return 1;

	return 0;
}