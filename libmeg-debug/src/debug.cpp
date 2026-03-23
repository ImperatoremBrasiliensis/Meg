#include <internal/debug.hpp>

#include <internal/common.hpp>

#include <cassert>
#include <csignal>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <mutex>
#include <vector>

// Debugger data.
static struct data {
	std::mutex mtx;
	std::vector<dbg::exception> e_stack;
	char buffer[256];
	bool enabled;
} data;

// Per thread data.
thread_local static struct thrd_data {
	std::vector<dbg::session*> s_stack;
} thrd_data;

// Signal handler.
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

// The `dbg::session` struct constructor.
dbg::session::session(bool enable, bool pers) noexcept {
	// Chooses randomly the `session` ID.
	id = []() -> uint16_t {
		static uint16_t lfsr = 1u;

		// Returns the LFSR; I don't know how this works exacly.
		return lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xb400u);
	}();

	// If another `session` has the same ID `goto error`.
	for (const session* e: thrd_data.s_stack)
		if (e->id == id)
			goto error;

	enabled = enable;
	pers_es = pers;
	thrd_data.s_stack.push_back(this);
	return;

error:
	// Turns the session invalid (`id = 0`).
	id = 0;
	enabled = false;
	pers_es = false;
}

// The `dbg::session` struct destructor.
dbg::session::~session() {
	if (!id)
		return;

	// If the session exceptions aren't persistent, remove them from main stack.
	if (!pers_es) {
		std::erase_if(
			data.e_stack,
			[this](const exception e) {
				return e.session_id == id;	  // Only if has the same ID.
			}
		);
	} else {
		for (exception &e: data.e_stack)
			if (e.session_id == id)
				e.session_id = 0;	 // Disassociate the exception.
	}

	data.e_stack.shrink_to_fit();
	e_stack.clear();

	// Removes this session from thread sessions stack.
	std::erase_if(
		thrd_data.s_stack,
		[this](session* s) {
			return s == this;
		}
	);
}

// Enable the `dbg::session` logging.
bool dbg::session::enable(bool v) noexcept {
	if (!id)
		return false;

	enabled = v;
	return true;
}

// Checks if the session is enabled.
bool dbg::session::is_enabled() noexcept {
	if (!id)
		return false;

	// if debugger is enabled, always return `true`.
	if (dbg::is_enabled())
		return true;

	return enabled;
}

// Get the last exception on the session stack.
dbg::exception dbg::session::get_exception() noexcept {
	if (!id)
		return {nullptr, NO_EXCEPTION};

	return e_stack.back();
}

// Get the exveption at `at`.
dbg::exception dbg::session::get_exception(size_t at) noexcept {
	if (!id)
		return {nullptr, NO_EXCEPTION};

	// If %at is out-of-range, it returns.
	if (!(at < e_stack.size()))
		return {nullptr, NO_EXCEPTION};

	return e_stack.at(at);
}

// Get all exceptions stored into the session exception stack.
std::vector<dbg::exception> dbg::session::get_exceptions() noexcept {
	if (!id)
		return {};

	return e_stack;
}

// Gets exceptions with a specific `exception_code`.
std::vector<dbg::exception> dbg::session::get_exceptions(exception_code code) noexcept {
	if (!id)
		return {};

	std::vector<exception> es;

	for (exception &e: e_stack)
		if (e.code == code)
			// If the code matches, stores it in %es.
			es.push_back(e);

	return es;
}

// Returns the size of the session exception stack.
size_t dbg::session::get_exception_count() noexcept {
	if (!id)
		return false;

	return e_stack.size();
}

// Throws an exceptin with the sesion ID.
bool dbg::session::throw_exception(exception e) noexcept {
	e.session_id = id;
	return dbg::throw_exception(e);
}

// Prints a formatted log message with detailed time information in the session.
bool dbg::session::log(uint8_t flags, const char* msg, ...) noexcept {
	// If the session or all debugger is disabled, returns `true`.
	if ((!dbg::is_enabled() && !enabled) || !id)
		return false;

	va_list va;
	va_start(va, msg);
	bool r = dbg::log(PERS | flags, msg, va);
	va_end(va);

	return r;
}

// Prints a formatted log message with detailed time information in the session.
bool dbg::session::log(uint8_t flags, const char* msg, va_list va) noexcept {
	// If the session or all debugger is disabled, returns `false`.
	if ((!dbg::is_enabled() && !enabled) || !id)
		return false;

	return dbg::log(PERS | flags, msg, va);
}

// Enables the debugger.
bool dbg::enable(bool enable [[maybe_unused]]) noexcept {
#ifdef DEBUG
	// Always return `false` if debug is compilation-enabled.
	return false;
#else
	data.enabled = enable;
	return true;
#endif
}

// Checks if debug is enabled.
bool dbg::is_enabled() noexcept {
#ifdef DEBUG
	// Always return true tf debug is always enabled.
	return true;
#else
	return data.enabled;
#endif
}

// Throws a `dbg::exception` an stores it in the exception stack (data.e_stack).
bool dbg::throw_exception(dbg::exception e) noexcept {
	// The exception `code` and `msg` must not be null.
	if (!e.code || !e.msg)
		return false;

	data.e_stack.push_back(e);

	// Updates all existing sessions with the new exception.
	for (session* s: thrd_data.s_stack)
		s->e_stack.push_back(e);	// Is per thread.

	return true;
}

// Gets the last exception.
dbg::exception dbg::get_exception() noexcept {
	return data.e_stack.back();
}

// Gets the exception at `at`.
dbg::exception dbg::get_exception(size_t at) noexcept {
	// If `at` is out-of-range, it returns an invalid.
	if (!(at < data.e_stack.size()))
		return {nullptr, NO_EXCEPTION};

	return data.e_stack.at(at);
}

// Get all exceptions stored into the exception stack.
std::vector<dbg::exception> dbg::get_exceptions() noexcept {
	return data.e_stack;
}

// Gets exceptions with a specific `dbg::exception_code` code.
std::vector<dbg::exception> dbg::get_exceptions(exception_code code) noexcept {
	std::vector<exception> es;

	for (exception &e: data.e_stack)
		if (e.code == code)
			// If the code matches, stores it in `es` to return.
			es.push_back(e);

	return es;
}

// Return the size of the exception stack.
size_t dbg::get_exception_count() noexcept {
	return data.e_stack.size();
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

// Function that choses the print/log prefix in `prefix` struct.
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

// Just a wrapper to `dbg::print` that receives a `va_list`.
// Prints the `msg` with the prefix based on `flags`.
bool dbg::print(uint8_t flags, const char* msg, ...) noexcept {
	va_list va_args;
	va_start(va_args, msg);
	bool r = print(flags, msg, va_args);
	va_end(va_args);
	return r;
}

// Prints the `msg` with the prfix based on `flags`.
bool dbg::print(uint8_t flags, const char* msg, va_list va) noexcept {
	FILE* stream = flags & FATA ? stderr : stdout;
	if (!msg)
		fputs("(null)", stream);

	const char* pfx = choose_prefix(flags);
	fputs(pfx, stream);
	vfprintf(stream, msg, va);

	return true;
}

// Prints a formatted log message, with detailed time information.
bool dbg::log(uint8_t flags, const char* msg, ...) noexcept {
	va_list va_args;
	va_start(va_args, msg);
	bool r = log(flags, msg, va_args);
	va_end(va_args);
	return r;
}

// Prints a log, but receives a `va_list`.
bool dbg::log(uint8_t flags, const char* msg, va_list va) noexcept {
	// If `msg` is nullptr, returns; or
	// If the flag `dbg::PERS` wasn't set AND debugger is not enabled, returns too.
	if (!msg || (!(flags & PERS) && !is_enabled()))
		return false;

	char buffer[256];
	size_t offset = 0;
	auto pfx = choose_prefix(flags);

	// Gets the time and prints the time information.
	time_t now = time(nullptr);
	struct tm lt = *(localtime(&now));
	offset += snprintf(
		buffer,
		sizeof(buffer),
		"\033[1;38;5;008m[%d/%d/%d § %d:%d:%d]\033[0m %s",
		lt.tm_mday,
		lt.tm_mon + 1,
		lt.tm_year + 1900,
		lt.tm_hour,
		lt.tm_min,
		lt.tm_sec,
		pfx
	);

	offset += vsnprintf(
		buffer + offset,
		sizeof(buffer),
		msg,
		va
	);

	static std::mutex mtx;
	mtx.lock();
	sys_write(2, buffer, offset);
	mtx.unlock();

	return true;
}
