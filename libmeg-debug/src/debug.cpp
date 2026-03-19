#include <debug.hpp>

#include <common.hpp>

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>

// Black-box.
namespace {
	// Whrere exceptions are stored.
	static std::vector<dbg::exception> e_stack{};

	// Debugger data.
	[[maybe_unused]] struct {
		bool debug_enabled;	   // Is debugger enabled?
	} data;

	// An structure containing the prefixes
	// that are used in print and log functions.
	constexpr struct {
		const char* fer = "\033[1mMeg \033[38;5;196mFatal Error: \033[0m";
		const char* err = "\033[1mMeg \033[38;5;196mError: \033[0m";
		const char* wrn = "\033[1mMeg \033[38;5;220mWarning: \033[0m";
		const char* dbg = "\033[1mMeg \033[38;5;200mDebug: \033[0m";
		const char* inf = "\033[1mMeg \033[38;5;040mInfo: \033[0m";
		const char* def = "\033[1mMeg: \033[0m";
	} prefix;

	// Function that choses the prefix in %prefix struct.
	in_line const char* choose_prefix(uint8_t flags) {
		if (flags & ERRO)
			if (flags & FATA)
				return prefix.fer;
			else
				return prefix.err;
		else if (flags & WARN)
			return prefix.wrn;
		else if (flags & DBUG)
			return prefix.dbg;
		else if (flags & INFO)
			return prefix.inf;
		else
			return prefix.def;
	}
}	 // namespace

// Actives the debugger.
bool dbg::active_debug(bool active [[maybe_unused]]) noexcept {
#ifdef DEBUG
	// Always return true if debug is always enabled.
	return false;
#else
	// If the debugger is already enabled or disabled, return false;
	if (data.debug_enabled == active)
		return false;

	// Sets the debugger active.
	data.debug_enabled = active;
	return true;	// Only return true if debugger is not %active.
#endif
}

bool dbg::is_debug_active() noexcept {
#ifdef DEBUG
	// Always return true if debugger is always enabled.
	return true;
#else
	// Returns the current state of debugger.
	return data.debug_enabled;
#endif
}

// Throws an $exception an stores it in the %e_stack.
bool dbg::throw_exception(dbg::exception e) noexcept {
	// %e.code and %e.msg must be diferent of 0 (dbg::NO_EXCEPTION and nullptr);
	if (!e.code || !e.msg)
		return false;	 // it returns false if not.

	// Stores %e into the exception stack e_stack.
	e_stack.push_back(e);
	return true;
}

// Gets the last exception.
dbg::exception dbg::get_exception() noexcept {
	return e_stack.back();
}

// Get the exception in a specific index, beginning by 0.
dbg::exception dbg::get_exception(size_t at) noexcept {
	// If %at is out-of-range, it returns.
	if (!(at < e_stack.size()))
		return {};

	// Returns the exception.
	return e_stack.at(at);
}

// Get all exceptions stored into the exception stack (%e_stack).
std::vector<dbg::exception> dbg::get_exceptions() noexcept {
	// Returns a copy of %e_stack.
	std::vector<exception> es = e_stack;
	return es;
}

// Gets exceptions with a specific $exception_code code.
std::vector<dbg::exception> dbg::get_exceptions(exception_code with_code) noexcept {
	// The $std::vector with the qualified exceotions.
	std::vector<exception> es;

	// Iterates in %e_stack and get exceptions with %with_code.
	for (exception &e: e_stack)
		if (e.code == with_code)
			// If the code matches, stores it in %es.
			es.push_back(e);

	// Returns es.
	return es;
}

// Return the size of the exception stack.
size_t get_exception_count() noexcept {
	return e_stack.size();	  // Returns the size.
}

// Prints the %msg with the prefix based on %flags.
bool dbg::print(uint8_t flags, const char* msg, ...) noexcept {
	// Starts the %... parameter, which contains the placeholders.
	va_list va_args;
	va_start(va_args, msg);

	// Calls %print function that receives a $va_list as parameter.
	bool r = print(flags, msg, va_args);

	// Ends %va_args and return r;
	va_end(va_args);
	return r;
}

// Prints the %msg with the prfix based on %flags, but
// Receives a va_list (%va).
bool dbg::print(uint8_t flags, const char* msg, va_list va) noexcept {
	// If %msg is nullptr, sets %msg as "(null)".
	if (!msg)
		msg = "(null)";

	// Chooses the prefix.
	const char* pfx = choose_prefix(flags);

	// Chooses the stream to print (%stderr or %stdout).
	auto stream = flags & FATA ? stderr : stdout;

	// Prints the prefix + format.
	fputs(pfx, stream);			  // The prefix.
	vfprintf(stream, msg, va);	  // The msg.

	// Returns true.
	return true;
}

// Prints a log, with detailed information.
bool dbg::log(uint8_t flags, const char* msg, ...) noexcept {
	// Starts the %... parameter, which contains the placeholders.
	va_list va_args;
	va_start(va_args, msg);

	// Calls %log function that receives a $va_list as parameter.
	bool r = log(flags, msg, va_args);

	// Ends %va_args and return r;
	va_end(va_args);
	return r;
}

// Prints a log, but receives a va_list (%va).
bool dbg::log(uint8_t flags, const char* msg, va_list va) noexcept {
	// If the %msg is nullptr, returns; or
	// If the flag dbg::PERS wasn't set AND debugger is not enabled, returns.
	if (!msg || (!(flags & PERS) && !is_debug_active()))
		return false;

	// Chposes the prefix.
	auto pfx = choose_prefix(flags);

	// Chooses the output (%stderr or %stdout)
	auto stream = flags & FATA ? stderr : stdout;

	// Gets the date and hour.
	time_t now = time(nullptr);			  // The time.
	struct tm lt = *(localtime(&now));	  // The local date and hour.

	// Prints the time info, prefix (pfx) and
	// the %msg + va (values to placeholders).
	fprintf(
		stream,
		"\033[1;38;5;008m[%d/%d/%d § %d:%d:%d]\033[0m ",
		lt.tm_mday,				  // da.
		lt.tm_mon + 1,			  // month - 1.
		lt.tm_year + 1900,		  // year (since 1900).
		lt.tm_hour,				  // hour.
		lt.tm_min,				  // minutes.
		lt.tm_sec				  // seconds.
	);							  // Time data.
	fputs(pfx, stream);			  // The prefix.
	vfprintf(stream, msg, va);	  // %msg formated with %va.

	// Returns true.
	return true;
}
