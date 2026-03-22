#include <cstdlib>
#include <internal/debug.hpp>

#include <internal/common.hpp>

#include <cassert>
#include <csignal>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>

// Debugger data.
static struct data {
	std::vector<dbg::exception> e_stack;
	char buffer[256];
	bool enabled;
} data;

// Per thread data.
thread_local static struct thrd_data {
	std::vector<dbg::session*> s_stack;
} thrd_data;

// Prints the handled signal.
static void handler(int signal) {
	switch (signal) {
	case SIGINT:
		dbg::print(FATA | DEFT, "Application interrupted. Exiting now...\n");
		exit(signal);
	case SIGABRT:
		dbg::print(FATA | ERRO, "Application aborting now!\n");
		exit(signal);
	case SIGSEGV:
		dbg::print(FATA | ERRO, "Segmentation fault. Exition now...\n");
		exit(signal);
	}
}

// Intializes the debugger.
bool dbg::init(bool enable) noexcept {
	// Sets debug enabled/disabled and sets the log buffer.
	data.enabled = enable;
	setvbuf(stdout, data.buffer, _IOFBF, sizeof(data.buffer));

	signal(SIGINT, handler);
	signal(SIGABRT, handler);
	signal(SIGSEGV, handler);

	return true;
}

// The `dbg::session` class constructor.
dbg::session::session(bool enable, bool pers) noexcept {
	// Chooses randomly the `seddion` ID.
	id = []() -> uint16_t {
		static uint16_t lfsr = 1u;	  // The LFSR.

		// Returns the LFSR; I don't know how this works.
		return lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xb400u);
	}();

	// If another `session` has the same ID `goto error`.
	for (const session* e: thrd_data.s_stack)
		if (e->id == id)
			goto error;

	enabled = enable;					  // Enable/disable the session log work.
	pers_es = pers;						  // If `true`, the session exceptions will never be removed from `dats.e_staack`.
	thrd_data.s_stack.push_back(this);	  // Registers on the thread session stack.
	goto exit;							  // Returns.

error:
	// Turns the session invalid (`id = 0`) and zero-initializes all.
	id = 0;
	enabled = false;
	pers_es = false;

// Substitute to `return` keyword.
exit:
}

// The `dbg::session` class destructor.
dbg::session::~session() {
	// If the session's exceptions aren't persistent, remove they from main stack.
	if (!pers_es) {
		// Erases from the `data.e_stack` if the exception has this session ID.
		std::erase_if(
			data.e_stack,
			[this](const exception e) {
				return e.session_id == id;
			}
		);
	} else {
		// If the exceptions aren't persistent, just disassociate the session.
		for (exception &e: data.e_stack)
			if (e.session_id == id)
				e.session_id = 0;	 // Turns the exeception ID 0 (no session).
	}

	data.e_stack.shrink_to_fit();	 // Shrink the main stack to fit.
	e_stack.clear();				 // Clear the session stack.

	// Finds this session on the thread session stack and erase it.
	std::erase_if(
		thrd_data.s_stack,
		[this](session* s) {
			return s == this;
		}
	);
}

// Enable the `dbg::session` logging.
bool dbg::session::enable(bool v) noexcept {
	// Checks if the session is valid.
	if (!id)
		return false;

	// If the session is already enabled/disable, return `false`.
	if (enabled == v)
		return false;

	// If the debugger is enabled, set session enabled as `v` and return `false`.
	if (dbg::is_enabled()) {
		enabled = v;
		return false;
	}

	// Set enabled/disabled and return false.
	enabled = v;
	return true;
}

// Checks if the session is enabled.
bool dbg::session::is_enabled() noexcept {
	// Checks if the session is valid.
	if (!id)
		return false;

	// if debugger is enabled, always return `true`.
	if (dbg::is_enabled())
		return true;

	// Return the value.
	return enabled;
}

// Get the last exception on the session stack.
dbg::exception dbg::session::get_exception() noexcept {
	// Checks if the session is valid.
	if (!id)
		return {nullptr, NO_EXCEPTION};

	return e_stack.back();
}

// Get the exveption at `at`.
dbg::exception dbg::session::get_exception(size_t at) noexcept {
	// Checks if the session is valid.
	if (!id)
		return {nullptr, NO_EXCEPTION};

	// If %at is out-of-range, it returns.
	if (!(at < e_stack.size()))
		return {nullptr, NO_EXCEPTION};

	// Returns the exception.
	return e_stack.at(at);
}

// Get all exceptions stored into the session exception stack.
std::vector<dbg::exception> dbg::session::get_exceptions() noexcept {
	// Checks if the session is valid.
	if (!id)
		return {};

	// Returns a copy of `e_stack`.
	return e_stack;
}

// Gets exceptions with a specific `exception_code`.
std::vector<dbg::exception> dbg::session::get_exceptions(exception_code code) noexcept {
	// Checks if the session is valid.
	if (!id)
		return {};

	// The `std::vector` with the qualified exceotions.
	std::vector<exception> es;

	// Iterates in %e_stack and get exceptions with %with_code.
	for (exception &e: e_stack)
		if (e.code == code)
			// If the code matches, stores it in %es.
			es.push_back(e);

	// Returns es.
	return es;
}

// Returns the size of the session exception stack.
size_t dbg::session::get_exception_count() noexcept {
	// Checks if the session is valid.
	if (!id)
		return false;

	return e_stack.size();	  // Returns the size.
}

// Throws an exceptin with the sesion ID.
bool dbg::session::throw_exception(exception e) noexcept {
	e.session_id = id;
	return dbg::throw_exception(e);
}

// Prints a formatted log message with detailed time information in the session.
bool dbg::session::log(uint8_t flags, const char* msg, ...) noexcept {
	// If the session or all debugger is disabled, returns `true`.
	if (!dbg::is_enabled() && !enabled && !id)
		return false;

	// Starts the `va_list` and calls `dbg::log` function with it.
	va_list va;
	va_start(va, msg);
	bool r = log(PERS | flags, msg, va);
	va_end(va);

	// Returns the log return value.
	return r;
}

// Prints a formatted log message with detailed time information in the session.
bool dbg::session::log(uint8_t flags, const char* msg, va_list va) noexcept {
	// If the session or all debugger is disabled, returns `true`.
	if (!dbg::is_enabled() && !enabled && !id)
		return false;

	//  Returns with a call to `dbg::log` function.
	return dbg::log(PERS | flags, msg, va);
}

// Actives the debugger.
bool dbg::enable(bool enable [[maybe_unused]]) noexcept {
#ifdef DEBUG
	// Always return `true` if debug is compilation-enabled.
	return false;
#else
	// If the debugger is already enabled or disabled, return `false`.
	if (data.enabled == enable)
		return false;

	// Sets dbeug active.
	data.enabled = enable;
	return true;	// Only return true if debugger was not enabled before.
#endif
}

// Checks if debug is enabled.
bool dbg::is_enabled() noexcept {
#ifdef DEBUG
	// Always return true tf debug is always enabled.
	return true;
#else
	// Returns the current state of debugger.
	return data.enabled;
#endif
}

// Throws a `dbg::exception` an stores it in the exception stack (data.e_stack).
bool dbg::throw_exception(dbg::exception e) noexcept {
	// The exception `code` and `msg` must be diferent of 0 (dbg::NO_EXCEPTION and nullptr);
	if (!e.code || !e.msg)
		return false;	 // it returns `false` if not.

	// Stores the exception into the exception stack.
	data.e_stack.push_back(e);

	// Updates all existing sessions with the exception.
	for (session* s: thrd_data.s_stack)
		s->e_stack.push_back(e);

	return true;
}

// Gets the last exception.
dbg::exception dbg::get_exception() noexcept {
	return data.e_stack.back();
}

// Get the exception at `at`.
dbg::exception dbg::get_exception(size_t at) noexcept {
	// If %at is out-of-range, it returns.
	if (!(at < data.e_stack.size()))
		return {nullptr, NO_EXCEPTION};

	// Returns the exception.
	return data.e_stack.at(at);
}

// Get all exceptions stored into the exception stack.
std::vector<dbg::exception> dbg::get_exceptions() noexcept {
	return data.e_stack;
}

// Gets exceptions with a specific `dbg::exception_code` code.
std::vector<dbg::exception> dbg::get_exceptions(exception_code code) noexcept {
	// A `std::vector` with the qualified exceotions.
	std::vector<exception> es;

	// Iterates in %e_stack and get exceptions with %with_code.
	for (exception &e: data.e_stack)
		if (e.code == code)
			// If the code matches, stores it in %es.
			es.push_back(e);

	// Returns es.
	return es;
}

// Return the size of the exception stack.
size_t dbg::get_exception_count() noexcept {
	return data.e_stack.size();	   // Returns the size.
}

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

	// Prints the prefix + format.
	fputs(pfx, stdout);	   // The prefix.
	vprintf(msg, va);	   // The msg.

	// Returns true.
	return true;
}

// Prints a formatted log message, with detailed time information.
bool dbg::log(uint8_t flags, const char* msg, ...) noexcept {
	// Starts the variadic parameters, which contains the placeholders value.
	va_list va_args;
	va_start(va_args, msg);

	// Calls `log` function that receives a `va_list` as parameter.
	bool r = log(flags, msg, va_args);

	// Ends `va_args` and return r;
	va_end(va_args);
	return r;
}

// Prints a log, but receives a `va_list`.
bool dbg::log(uint8_t flags, const char* msg, va_list va) noexcept {
	// If `msg` is nullptr, returns; or
	// If the flag `dbg::PERS` wasn't set AND debugger is not enabled, returns too.
	if (!msg || (!(flags & PERS) && !is_enabled()))
		return false;

	// Creates a buffer and chooses the prefix.
	char buffer[256];
	size_t offset = 0;
	auto pfx = choose_prefix(flags);

	// Gets the time.
	time_t now = time(nullptr);			  // The time.
	struct tm lt = *(localtime(&now));	  // The local date and hour.

	// Prints the time information and the prefix.
	offset += snprintf(
		buffer,
		sizeof(buffer),
		"\033[1;38;5;008m[%d/%d/%d § %d:%d:%d]\033[0m %s",
		lt.tm_mday,			  // day.
		lt.tm_mon + 1,		  // month - 1.
		lt.tm_year + 1900,	  // year (since 1900).
		lt.tm_hour,			  // hour.
		lt.tm_min,			  // minutes.
		lt.tm_sec,			  // seconds.
		pfx					  // prefix.
	);

	// Prints the formatted message.
	offset += vsnprintf(
		buffer + offset,
		sizeof(buffer),
		msg,
		va
	);

	// Prints the buffer on `stdout` or `stderr`.
	fwrite(
		buffer,
		offset,
		1,
		flags & FATA ? stderr : stdout
	);

	// Returns true
	return true;
}
