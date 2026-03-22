#pragma once

/* Qualifiers */

#if __has_attribute(always_inline)
#	define in_line __attribute((always_inline))
#else
#	define in_line inline
#endif

#if __has_attribute(visibility)
#	define extra __attribute((visibility("default")))
#endif
