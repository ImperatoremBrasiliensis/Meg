/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#pragma once

#include <cstdint>
#include <string>

namespace Meg::lang {
	bool init(const char *lang_dir_path) noexcept;

	typedef enum : uint16_t {
		LANG_NONE,
		LANG_LA,
		LANG_POR_BR
	} language;

	bool load_lang(language) noexcept;

	const char *get_key(const char *key) noexcept;

	std::string get_key(std::string key) noexcept;
}	 // namespace Meg::lang