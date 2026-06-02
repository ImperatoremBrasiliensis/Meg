/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#pragma once

#include <vector>

namespace compiler {
	extern std::vector<const char *> sources;

	int compile(int argc, char *arg[]) noexcept;
}	 // namespace compiler
