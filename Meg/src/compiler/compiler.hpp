#pragma once

/* STD */
#include <vector>

namespace compiler {
	extern std::vector<const char*> sources;

	int compile(int argc, char* arg[]) noexcept;
}    // namespace compiler