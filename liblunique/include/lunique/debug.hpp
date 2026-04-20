#pragma once

#include <lunique/common.hpp>

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <source_location>
#include <vector>

#define LUNIQUE_SET_ALIAS(name, func, flags) \
in_line constexpr bool name(const char* msg, auto ...va) noexcept { \
	return func(flags, msg, va...); \
}

#define LUNIQUE_DEFT (uint8_t) 0b00000000
#define LUNIQUE_ERRO (uint8_t) 0b00000001
#define LUNIQUE_WARN (uint8_t) 0b00000010
#define LUNIQUE_DBUG (uint8_t) 0b00000100
#define LUNIQUE_INFO (uint8_t) 0b00001000
#define LUNIQUE_FATA (uint8_t) 0b00010000
#define LUNIQUE_PERS (uint8_t) 0b00100000

/**
 * @brief The debug system, which can be used to print messages and logs,
 * throw and get exceptions and control program runtime.
 */
namespace lunique::dbg {
	/**
	 * @brief Initializes the debugger.
	 * 
	 * @param active Defines if the debugger is actives or not.
	 *
	 * @return `true` if success; `false` if not.
	 */
	bool init(bool active) noexcept;

	/**
	 * @brief An enum with the all exception ( @ref exception )
	 * codes possible.
	 */
	typedef enum : uint16_t {
		NO_EXCEPTION,
		ALLOC_FAULT,
		BUFFER_OVERFLOW,
		INVALID_ARGUMENT,
		NULL_POINTER,
		OUT_OF_RANGE,
	} exception_code;

	/**
	 * @brief Is a struct specifying an exception
	 * with a message explaining it.
	 */
	struct exception {
		const char *func_name;
		int line;
		const char *msg;
		exception_code code;

		/**
		 * @brief Construct a new `exception` object.
		 * 
		 * @param msg Defines a message to 
		 * @param code 
		 */
		exception(
			const char *msg,
			exception_code code,
			std::source_location meta = std::source_location::current()
		) noexcept:
				func_name(meta.function_name()),
				line(meta.line()),
				msg(msg),
				code(code) {
		}

	private:
		uint16_t session_id = 0;

		friend struct session;
	};

	/**
	 * @brief A way to debug some parts of code.
	 * 
	 */
	struct session {
	private:
		std::vector<exception> e_stack{};
		uint16_t id;
		bool enabled:1, pers_es:1;

		friend bool throw_exception(dbg::exception e) noexcept;
		friend bool throw_exception(const char *msg, exception_code code, std::source_location) noexcept;

	public:
		/**
		 * @brief Construct a new session object.
		 * 
		 * @param enaable Sets if the new session is 
		 * active or not.
		 *
		 * @param pers Sets if the exceptions of the
		 * session must remain after its destruction 
		 * or not.
		 */
		session(bool enable, bool pers) noexcept;

		/**
		 * @brief Destroy the session object.
		 * 
		 */
		~session();

		/**
		 * @brief Enables/disables the session, allowing you
		 * to print log messages with `dbg::session::log` 
		 * function.
		 * 
		 * @param v If `true`, the session will be enabled, if
		 * `false`, the session will be disabled.
		 *
		 * @return If the session is invalid, it returns `false`,
		 * if not it returns `true`.
		 */
		bool enable(bool v) noexcept;

		/**
		 * @brief Checks if the session is enabled.
		 * 
		 * @return `true` if the session is enabled; 
		 * `false` if not.
		 *
		 * @warning If debugger is enabled, __it will
		 * always return `true`__.
		 */
		[[nodiscard]]
		bool is_enabled() noexcept;

		/** 
		 * @brief Throws an `dbg::exception` and puts
		 * it in the session and main exception stack.
		 * 
		 * @param e An `exception` to throw.
		 *
		 * @returns `true` if the function call was
		 * succesful; `false` if not.
		 */
		bool throw_exception(exception e) noexcept;

		/**
		 * @brief Throws a `dbg::exception` and stores
		 * it in the session and main exception stack.
		 * 
		 * @param msg The exception message.
		 * @param code The exception code.
		 *
		 * @return `true` if successful; `false` if not. 
		 */
		bool throw_exception(
			const char *msg,
			exception_code code,
			std::source_location = std::source_location::current()
		) noexcept;

		/**
		 * @brief Gets the last `dbg::exception` in the
		 * _session_ exception stack.
		 *
		 * @returns The last `dbg::exception` in the 
		 * exception stack.
		 */
		[[nodiscard]]
		exception get_exception() noexcept;

		/**
		 * @brief Gets the `dbg::exception` at @p at posistion
		 * in the _session_ exception stack.
		 * 
		 * @param at The index of the exception, beginning
		 * with 0;
		 * 
		 *	@warning The @p at param __must__ be equal or bigger than 0 and
		 * less than exception count ( @ref get_exception_count() ) - 1.
		 * 
		 * @return The exception at @p at ; an invalid `dbg::exception` 
		 * object will be returned.
		 */
		[[nodiscard]]
		dbg::exception get_exception(size_t at) noexcept;

		/**
		 * @brief Get all `dbg::exception`s in the session stack.
		 * 
		 * @return all exceptions stacked along session
		 * life. If the function failed, a zero-initialized
		 * vector will be returned.
		 */
		[[nodiscard]]
		std::vector<dbg::exception> get_exceptions() noexcept;

		/**
		 * @brief Gets all `dbg::exception`s in the session stack
		 * with the `code` field equal to @p code parameter.
		 * 
		 * @param code Value with which to find exceptions
		 * in the stack.
		 *
		 * @return A vector with all exceptions in the stack 
		 * with @p code value as code. If the functuon failed,
	 	* a zero-initialized vector will be returned.
		 */
		[[nodiscard]]
		std::vector<dbg::exception> get_exceptions(exception_code code) noexcept;

		/**
		 * @brief Gets the count of `dbg::exception`s in
		 * the _session_ exception stack.
		 * 
		 * @return The exception stack size.
		 */
		[[nodiscard]]
		size_t get_exception_count() noexcept;

		/**
		 * @brief Prints a formatted log message with
		 * detailed time information.
		 * 
		 * @param flags Defines how function will behave
		 * and how message will be printed, possible flags are:
		 * `LUNIQUE_PERS` (persistent), `LUNIQUE_FATA` (fatal), `LUNIQUE_ERRO` (error),
		 * `LUNIQUE_DBUG` (debug), `LUNIQUE_INFO` (information) and `LUNIQUE_DEFT` (default).
		 *
		 * @details The message prefix will be printed according
		 * to @p flags parameter. Exceptionally, if `LUNIQUE_FATA` was wet 
		 * with `LUNIQUE_ERRO`, the prefix will be `Meg Fatal Error`.
		 * If `LUNIQUE_PERS` was set, the log message will always be printed,
		 * until with debugger disabled.
		 * `LUNIQUE_DEFT` makes that message prefix be only `Meg`.
		 *
		 * @param msg The message to be printed into the stream. Should
		 * not be `nullptr`.
		 *
		 * @return `true` if the function was succesful; `false` if
		 * not.
		 */
		bool log(uint8_t flags, const char *msg, ...) noexcept;

		/**
		 * @brief Prints a formatted log message with
		 * detailed time information.
		 * 
		 * @param flags Defines how function will behave
		 * and how message will be printed, possible flags are:
		 * `LUNIQUE_PERS` (persistent), `LUNIQUE_FATA` (fatal), `LUNIQUE_ERRO` (error),
		 * `LUNIQUE_DBUG` (debug), `LUNIQUE_INFO` (information) and `LUNIQUE_DEFT` (default).
		 *
		 * @details The message prefix will be printed according
		 * to @p flags parameter. Exceptionally, if `LUNIQUE_FATA` was wet 
		 * with `LUNIQUE_ERRO`, the prefix will be `Meg Fatal Error`.
		 * If `LUNIQUE_PERS` was set, the log message will always be printed,
		 * until with debugger disable.
		 * `LUNIQUE_DEFT` makes that message prefix be only `Meg`.
		 *
		 * @param msg The message to be printed into the stream. Should
		 * not be `nullptr`.
		 *
		 * @param va A started `va_list` with values to
		 * placeholders in the @p msg string.
		 *
		 * @return `true` if the function was succesful; `false` if
		 * not.
		 */
		bool log(uint8_t flags, const char *msg, va_list va) noexcept;

		LUNIQUE_SET_ALIAS(log_ferr, log, LUNIQUE_PERS | LUNIQUE_FATA | LUNIQUE_ERRO);
		LUNIQUE_SET_ALIAS(log_err, log, LUNIQUE_ERRO);
		LUNIQUE_SET_ALIAS(log_war, log, LUNIQUE_WARN);
		LUNIQUE_SET_ALIAS(log_deb, log, LUNIQUE_DBUG);
		LUNIQUE_SET_ALIAS(log_inf, log, LUNIQUE_INFO);
		LUNIQUE_SET_ALIAS(log_def, log, LUNIQUE_DEFT);

		LUNIQUE_SET_ALIAS(log_pferr, log, LUNIQUE_PERS | LUNIQUE_FATA | LUNIQUE_ERRO);
		LUNIQUE_SET_ALIAS(log_perr, log, LUNIQUE_PERS | LUNIQUE_ERRO);
		LUNIQUE_SET_ALIAS(log_pwar, log, LUNIQUE_PERS | LUNIQUE_WARN);
		LUNIQUE_SET_ALIAS(log_pdeb, log, LUNIQUE_PERS | LUNIQUE_DBUG);
		LUNIQUE_SET_ALIAS(log_pinf, log, LUNIQUE_PERS | LUNIQUE_INFO);
		LUNIQUE_SET_ALIAS(log_pdef, log, LUNIQUE_PERS | LUNIQUE_DEFT);
	};
	/**
	 * @brief Sets debugger enabled.
	 * 
	 * @details Sets debugger enabled or disabled,
	 * which is essential for `dbg::log` function
	 * call ( @ref log ). If the call to `dbg::log` 
	 * has the `LUNIQUE_PERS` (persistence) flag, you have
	 * nothing to worry about.
	 *
	 * @param active If `true`, debugging will
	 * be enabled; if `false`, debugging will be 
	 * disabled.
	 *
	 * @return `true` if successful; false if debugging
	 * was compilation enabled.
	 */
	bool enable(bool enable) noexcept;

	/**
	 * @brief Checks if debugging is enabled or not.
	 *
	 * @warning If the marcro `DEBUG` was set in the
	 * compilation-time, this function will __always__ return 
	 * `true`.
	 *
	 * @return `true` if the debugging is enabled; 
	 * `false` if not.
	 */
	[[nodiscard]]
	bool is_enabled() noexcept;

	/** 
	 * @brief Throws a `dbg::exception` and stores
	 * it in the exception stack.
	 * 
	 * @param e A `dbg::exception` object to throw.
	 *
	 * @note All existing session in the thread will
	 * be _updated_ with this session.
	 *
	 * @returns `true` if the function was succesful;
	 * `false` if not.
	 */
	bool throw_exception(exception e) noexcept;

	/**
	 * @brief Throws a `dbg::exception` and stores
	 * it in the exception stack.
	 * 
	 * @param msg The exception message.
	 * @param code The exception code.
	 *
	 * @return `true` if successful; `false` if not. 
	 */
	bool throw_exception(
		const char *msg,
		exception_code code,
		std::source_location = std::source_location::current()
	) noexcept;

	/**
	 * @brief Gets the last `dbg::exception` in the
	 * exception stack.
	 *
	 * @returns The last exception in the 
	 * exception stack.
	 */
	[[nodiscard]] exception get_exception() noexcept;

	/**
	 * @brief Gets the `dbg::exception` at @p at posistion
	 * in the exception stack.
	 * 
	 * @param at The index of the exception, beginning
	 * with 0;
	 * 
	 *	@warning The @p at param __must__ be equal or bigger than 0 and
	 * less than exception count ( @ref get_exception_count() ) - 1.
	 * 
	 * @return The exception at @p at ; an invalid `dbg::exception` 
	 * object will be returned.
	 */
	[[nodiscard]]
	dbg::exception get_exception(size_t at) noexcept;

	/**
	 * @brief Get all `dbg::exception`s in the stack.
	 * 
	 * @return all `exception`s stacked along debugger
	 * execution. If the function failed,
	 * a zero-initialized vector will be returned.
	 */
	[[nodiscard]]
	std::vector<dbg::exception> get_exceptions() noexcept;

	/**
	 * @brief Gets all `dbg::exception`s in the stack with the
	 * `code` field equal to @p code parameter.
	 * 
	 * @param code Value with which to find exceptions
	 * in the stack.
	 *
	 * @return A vector with all exceptions in the stack 
	 * with `code` field equal to @p code parameter. If
	 * the function failed, a zero-initialized vector
	 * will be returned.
	 */
	[[nodiscard]]
	std::vector<dbg::exception> get_exceptions(exception_code code) noexcept;

	/**
	 * @brief Gets the count of `dbg::exception`s in
	 * the exception stack.
	 * 
	 * @return The exception stack size.
	 */
	[[nodiscard]]
	size_t get_exception_count() noexcept;

	/**
	 * @brief Prints a formatted log message with
	 * detailed time information.
	 * 
	 * @param flags Defines how function will behave
	 * and how message will be printed, possible flags are:
	 * `LUNIQUE_PERS` (persistent), `LUNIQUE_FATA` (fatal), `LUNIQUE_LUNIQUE_ERRO` (error),
	 * `LUNIQUE_DBUG` (debug), `LUNIQUE_INFO` (information) and `LUNIQUE_DEFT` (default).
	 *
	 * @details The message prefix will be printed according
	 * to @p flags parameter. Exceptionally, if `LUNIQUE_FATA` was wet 
	 * with `LUNIQUE_ERRO`, the prefix will be `Meg Fatal Error`.
	 * If `LUNIQUE_PERS` was set, the log message will always be printed,
	 * until with debugger disabled.
	 * `LUNIQUE_DEFT` makes that message prefix be only `Meg`.
	 *
	 * @param msg The message to be printed into the stream. Should
	 * not be `nullptr`.
	 *
	 * @return `true` if the function was succesful; `false` if
	 * not.
	 */
	bool log(uint8_t flags, const char *msg, ...) noexcept;

	/**
	 * @brief Prints a message with a prefix based on the
	 * @p flags parameter and formatted with placeholders,
	 * which values are placed in variadic parameters.
	 *
	 * @param flags Defines how function will behave
	 * and how message will be printed, possible flags are:
	 * `LUNIQUE_FATA` (fatal), `LUNIQUE_ERRO` (error), `LUNIQUE_DBUG` (debug),
	 * `LUNIQUE_INFO` (information) and `LUNIQUE_DEFT` (default).
	 *
	 * @param msg A string containing the message to be 
	 * printed into the output.
	 *
	 * @warning The @p msg param. Must not be null, if contrary
	 * the function will print "(null)".
	 *
	 * @details The message prefix will be printed according
	 * to @p flags parameter. Exceptionally, if `LUNIQUE_FATA` was set 
	 * with `LUNIQUE_ERRO`, the prefix will be `Meg Fatal Error`. `LUNIQUE_FATA` 
	 * also makes that the message will be printed to `stderr`.
	 * `LUNIQUE_DEFT` makes that message prefix will be only `Meg`. 
	 * this allows you print messages to the output cleanly.
	 *
	 * @return `true` if the function was succesful; and
	 * `false` if not.
	 */
	bool print(uint8_t flags, const char *msg, ...) noexcept;

	/**
	 * @brief Prints a message with a prefix based on the
	 * @p flags parameter and formatted with placeholders,
	 * which values are placed in variadic parameters.
	 *
	 * @param flags Defines how function will behave
	 * and how message will be printed, possible flags are:
	 * `LUNIQUE_FATA` (fatal), `LUNIQUE_ERRO` (error), `LUNIQUE_DBUG` (debug),
	 * `LUNIQUE_INFO` (information) and `LUNIQUE_DEFT` (default).
	 *
	 * @param msg A string containing the message to be 
	 * printed into the output.
	 *
	 * @param va A started `va_list` with values to
	 * placeholders in the @p msg string.
	 *
	 * @warning The @p msg param. Must not be null, if contrary
	 * the function will print "(null)".
	 *
	 * @details The message prefix will be printed according
	 * to @p flags parameter. Exceptionally, if `LUNIQUE_FATA` was set 
	 * with `LUNIQUE_ERRO`, the prefix will be `Meg Fatal Error`. `LUNIQUE_FATA` 
	 * also makes that the message will be printed to `stderr`.
	 * `LUNIQUE_DEFT` makes that message prefix will be only `Meg`. 
	 * this allows you print messages to the output cleanly.
	 *
	 * @return `true` if the function was succesful; and
	 * `false` if not.
	 */
	bool print(uint8_t flags, const char *msg, va_list va) noexcept;

	LUNIQUE_SET_ALIAS(prt_ferr, print, LUNIQUE_PERS | LUNIQUE_FATA | LUNIQUE_ERRO);
	LUNIQUE_SET_ALIAS(prt_err, print, LUNIQUE_ERRO);
	LUNIQUE_SET_ALIAS(prt_war, print, LUNIQUE_WARN);
	LUNIQUE_SET_ALIAS(prt_deb, print, LUNIQUE_DBUG);
	LUNIQUE_SET_ALIAS(prt_inf, print, LUNIQUE_INFO);
	LUNIQUE_SET_ALIAS(prt_def, print, LUNIQUE_DEFT);

	LUNIQUE_SET_ALIAS(log_ferr, log, LUNIQUE_PERS | LUNIQUE_FATA | LUNIQUE_ERRO);
	LUNIQUE_SET_ALIAS(log_err, log, LUNIQUE_ERRO);
	LUNIQUE_SET_ALIAS(log_war, log, LUNIQUE_WARN);
	LUNIQUE_SET_ALIAS(log_deb, log, LUNIQUE_DBUG);
	LUNIQUE_SET_ALIAS(log_inf, log, LUNIQUE_INFO);
	LUNIQUE_SET_ALIAS(log_def, log, LUNIQUE_DEFT);

	LUNIQUE_SET_ALIAS(log_pferr, log, LUNIQUE_PERS | LUNIQUE_FATA | LUNIQUE_ERRO);
	LUNIQUE_SET_ALIAS(log_perr, log, LUNIQUE_PERS | LUNIQUE_ERRO);
	LUNIQUE_SET_ALIAS(log_pwar, log, LUNIQUE_PERS | LUNIQUE_WARN);
	LUNIQUE_SET_ALIAS(log_pdeb, log, LUNIQUE_PERS | LUNIQUE_DBUG);
	LUNIQUE_SET_ALIAS(log_pinf, log, LUNIQUE_PERS | LUNIQUE_INFO);
	LUNIQUE_SET_ALIAS(log_pdef, log, LUNIQUE_PERS | LUNIQUE_DEFT);
};	  // namespace lunique::dbg