/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#pragma once

#define MEG_TO_STRING(str) #str
#define MEG_MACRO_TO_STRING(macro) MEG_TO_STRING(macro)

#define MEG_ALIGN(n, value) ((n + value - 1) & ~(value - 1))

// For future support for Windows.
#ifdef _WIN32
#else
#	include <unistd.h>
#
#	define MEG_INLINE __attribute((always_inline)) inline
#	define MEG_EXTRA __attribute((visibility("default")))
#endif

#include <atomic>
#include <cstdlib>
#include <thread>

namespace Meg::common {
	class mutex {
		std::thread::id owner;
		std::atomic<bool> locked;

	public:
		void lock() noexcept;

		void unlock() noexcept;

		mutex() noexcept;

		mutex(bool lock) noexcept;

		~mutex() = default;
	};

#ifdef _WIN32
#else
#	include <unistd.h>
#

	MEG_INLINE size_t sys_write(const char *buf, size_t n) noexcept {
		static mutex sys_write_mtx;
		sys_write_mtx.lock();

		size_t r = write(STDOUT_FILENO, buf, n);

		sys_write_mtx.unlock();
		return r;
	}

#endif
};	  // namespace Meg::common