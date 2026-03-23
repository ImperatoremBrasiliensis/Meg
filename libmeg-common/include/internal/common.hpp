#pragma once

// For future support for Windows.
#ifdef _WIN32

#else
#	include <unistd.h>
#
#	define sys_write(fd, buf, n) write(fd, buf, n)
#
#	define in_line __attribute((always_inline))
#	define extra __attribute((visibility("default")))
#endif
