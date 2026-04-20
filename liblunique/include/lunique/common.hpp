#pragma once

// For future support for Windows.
#ifdef _WIN32
#
#	include <cstdlib>
#
#	define full_path(str, resolved, size) _fullpath(resolved, str, size)
#
#else
#	include <unistd.h>
#
#	define full_path(str, resolved, size) realpath(str, resolved)
#
#	define access_file(path) access(path, F_OK)
#
#	define in_line __attribute((always_inline)) inline
#	define extra __attribute((visibility("default")))
#endif

#include <atomic>
#include <cstdlib>
#include <thread>

namespace lunique::common {
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

	inline mutex sys_write_mtx;

#ifdef _WIN32
#else
#	include <unistd.h>
#

	in_line size_t sys_write(const char *buf, size_t n) noexcept {
		sys_write_mtx.lock();

		size_t r = write(STDOUT_FILENO, buf, n);

		sys_write_mtx.unlock();
		return r;
	}

#endif
};	  // namespace lunique::common