/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <Debug/debug.hpp>

using namespace Meg;

void dbg::panic(std::string_view msg, std::source_location loc) {
	std::cerr
		<< "\n\033[1mIn file: \033[0m" << loc.file_name()
		<< ":" << loc.line() << ", " << loc.column() << ":\n"
		<< "\033[1mIn function: \033[0m" << loc.function_name() << ":\n";
	log("\033[1;31mPanic\033[0m", msg);
	abort();
}
