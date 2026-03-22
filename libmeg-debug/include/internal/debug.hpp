#pragma once

#include <internal/common.hpp>

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <vector>

#define log_alias(name, flags) \
in_line constexpr bool name(const char* msg, ...) noexcept { \
	va_list va; \
	va_start(va, msg); \
	bool r = log(flags, msg, va); \
	va_end(va); \
	return r; \
} \
\
in_line constexpr bool name(const char* msg, va_list va) noexcept { \
	return log(flags, msg, va); \
}

#define DEFT (uint8_t) 0b00000000
#define ERRO (uint8_t) 0b00000001
#define WARN (uint8_t) 0b00000010
#define DBUG (uint8_t) 0b00000100
#define INFO (uint8_t) 0b00001000
#define FATA (uint8_t) 0b00010000
#define PERS (uint8_t) 0b00100000

/**
 * @brief The debug system, which can be used to print messages and logs
 * throw and get exceptions and control program runtime.
 * 
 */
namespace dbg {
	/**
	 * @brief Initializes the debugger.
	 * 
	 * @param active Defines if the debugger is actives or not.
	 * @return `true` if success; `false` if not.
	 */
	bool init(bool active) noexcept;

	/**
	 * @brief An enum with the all exception ( @ref exception )
	 * codes possible.
	 */
	typedef enum : uint16_t {
		NO_EXCEPTION,
		UNO_E,
		DUNO_E,
		TITRE_E
	} exception_code;

	/**
	 * @brief Is a struct specifying an exception
	 * and a message explaining it.
	 */
	struct exception {
		const char* msg;
		exception_code code;

		exception(const char* msg, exception_code code) noexcept:
			msg(msg),
			code(code) {
		}

	private:
		uint16_t session_id = 0;

		friend struct session;
	};

	struct session {
	private:
		std::vector<exception> e_stack{};
		uint16_t id;
		bool enabled:1, pers_es:1;

		friend bool throw_exception(dbg::exception e) noexcept;

	public:
		/**
		 * @brief Construct a new session object.
		 * 
		 * @param active Sets if the new session is 
		 * active.
		 *
		 * @param pers Sets if the exceptions of the
		 * session will remain after its destruction. 
		 */
		session(bool active, bool pers) noexcept;

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
		 * @return If the session is already @p v or debugger is
		 * `true` it returns `false`. If contrary it returns `true`.
		 */
		bool enable(bool v) noexcept;

		/**
		 * @brief Checks if session is enabled.
		 * 
		 * @return `true` if the session is enabled; 
		 * `false` if not.
		 *
		 * @warning If debugger is enabled, it will
		 * always return `true`.
		 */
		[[nodiscard]]
		bool is_enabled() noexcept;

		/** 
		 * @brief Throws an `exception` and stores
		 * it in the session exception stack.
		 * 
		 * @param e An `exception` to throw.
		 * @returns `true` if the function was succesful;
		 * `false` if not.
		 */
		bool throw_exception(exception e) noexcept;

		/**
		 * @brief Gets the last `exception` in the exception stack.
		 *
		 * @returns The last `exception` in the 
		 * exception stack.
		 */
		[[nodiscard]]
		exception get_exception() noexcept;

		/**
		 * @brief Gets the `exception` at @p at posistion.
		 * 
		 * @param at The index of the `exception`, beginning
		 * with 0;
		 * 
		 *	@warning The @p at param must be equal or bigger than 0 and
		 * less than exception count ( @ref get_exception_count() ) - 1.
		 * 
		 * @return an `eception` at @p at ; nothing if an
		 * occurred. 
		 */
		[[nodiscard]]
		dbg::exception get_exception(size_t at) noexcept;

		/**
		 * @brief Get all `exception`s in the session stack.
		 * 
		 * @return all `exception`s stacked along session
		 * life. If the functuon failed, a zero-initialized
		 * vector will be returned.
		 */
		[[nodiscard]]
		std::vector<dbg::exception> get_exceptions() noexcept;

		/**
		 * @brief Gets all `exception`s in the session stack with the
		 * code (`exception.code`) equal to @p code parameter.
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
		 * @brief Gets the count of `exception`s in
		 * the stack exception.
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
		 * `PERS` (persistent), `FATA` (fatal), `ERRO` (error),
		 * `DBUG` (debug), `INFO` (information) and `DEFT` (default).
		 *
		 * @details The message prefix will be printed according
		 * to @p flags parameter. Exceptionally, if `FATA` was wet 
		 * with `ERRO`, the prefix will be `Meg Fatal Error`. `FATA` 
		 * also makes that the log will be printed to `stderr`.
		 * If `PERS` was set, the log message will always be printed,
		 * until with debugger disable.
		 * `DEFT` maskes that message prefix will be only `Meg`.
		 *
		 * @param msg The message to be printed on the stream. Should
		 * not be nullptr.
		 *
		 * @return `true` if the function was succesful; `false` if
		 * not.
		 */
		bool log(uint8_t flags, const char* msg, ...) noexcept;

		/**
		 * @brief Prints a formatted log message with
		 * detailed time information.
		 * 
		 * @param flags Defines how function will behave
		 * and how message will be printed, possible flags are:
		 * `PERS` (persistent), `FATA` (fatal), `ERRO` (error),
		 * `DBUG` (debug), `INFO` (information) and `DEFT` (default).
		 *
		 * @details The message prefix will be printed according
		 * to @p flags parameter. Exceptionally, if `FATA` was wet 
		 * with `ERRO`, the prefix will be `Meg Fatal Error`. `FATA` 
		 * also makes that the log will be printed to `stderr`.
		 * If `PERS` was set, the log message will always be printed,
		 * until with debugger disable.
		 * `DEFT` maskes that message prefix will be only `Meg`.
		 *
		 * @param msg The message to be printed on the stream. Should
		 * not be nullptr.
		 *
		 * @param va A started `va_list` with values to
		 * placeholders in the @p msg string.
		 *
		 * @return `true` if the function was succesful; `false` if
		 * not.
		 */
		bool log(uint8_t flags, const char* msg, va_list va) noexcept;

		log_alias(err, ERRO);
		log_alias(war, WARN);
		log_alias(deb, DBUG);
		log_alias(inf, INFO);
		log_alias(def, DEFT);

		log_alias(perr, PERS | ERRO);
		log_alias(pwar, PERS | WARN);
		log_alias(pdeb, PERS | DBUG);
		log_alias(pinf, PERS | INFO);
		log_alias(pdef, PERS | DEFT);

		log_alias(ferr, FATA | ERRO);
		log_alias(fwar, FATA | WARN);
		log_alias(fdeb, FATA | DBUG);
		log_alias(finf, FATA | INFO);
		log_alias(fdef, FATA | DEFT);

		log_alias(pferr, PERS | FATA | ERRO);
		log_alias(pfwar, PERS | FATA | WARN);
		log_alias(pfdeb, PERS | FATA | DBUG);
		log_alias(pfinf, PERS | FATA | INFO);
		log_alias(pfdef, PERS | FATA | DEFT);
	};

	/**
	 * @brief Sets debugging enabled.
	 * 
	 * @details Sets debugger enable or disabled,
	 * which is essential for `dbg::log` function
	 * call ( @ref log ). If the call to `dbg::log` 
	 * has the `PERS` (persistence) flag, you have
	 * nothing to worry about.
	 *
	 * @param active If it's `true`, debugging will
	 * be enabled; if `false`, debuggig will be 
	 * disabled.
	 *
	 * @return According to @p enable parameter, `true` if debugging was
	 * enabled/disabled; `false` if debugging is already
	 * enabled/disabled.
	 */
	bool enable(bool enable) noexcept;

	/**
	 * @brief Checks if debugging is enabled or not.
	 *
	 * @warning If the marcro `DEBUG` was set in the
	 * compilation-time, this function will always return 
	 * `true`.
	 *
	 * @return `true` if the debugging is enabled; 
	 * `false` if not.
	 */
	[[nodiscard]]
	bool is_enabled() noexcept;

	/** 
	 * @brief Throws an `exception` and stores
	 * it in the exception stack.
	 * 
	 * @param e An `exception` to throw.
	 * @returns `true` if the function was succesful;
	 * `false` if not.
	 */
	bool throw_exception(exception e) noexcept;

	/**
	 * @brief Gets the last `exception` in the exception stack.
	 *
	 * @returns The last `exception` in the 
	 * exception stack.
	 */
	[[nodiscard]]
	exception get_exception() noexcept;

	/**
	 * @brief Gets the `exception` at @p at posistion.
	 * 
	 * @param at The index of the `exception`, beginning
	 * with 0;
	 * 
	 *	@warning The @p at param must be equal or bigger than 0 and
	 * less than exception count ( @ref get_exception_count() ) - 1.
	 * 
	 * @return an `eception` at @p at ; nothing if an
	 * occurred. 
	 */
	[[nodiscard]]
	dbg::exception get_exception(size_t at) noexcept;

	/**
	 * @brief Get all `exception`s in the stack.
	 * 
	 * @return all `exception`s stacked along debugger
	 * execution. If the functuon failed,
	 * a zero-initialized vector will be returned.
	 */
	[[nodiscard]]
	std::vector<dbg::exception> get_exceptions() noexcept;

	/**
	 * @brief Gets all `exception`s in the stack with the
	 * code (`exception.code`) equal to `code` parameter.
	 * 
	 * @param code Value with which to find exceptions
	 * in the stack.
	 *
	 * @return A vector with all exceptions in the stack 
	 * with @p code value as code. If the function failed,
	 * a zero-initialized vector will be returned.
	 */
	[[nodiscard]]
	std::vector<dbg::exception> get_exceptions(exception_code code) noexcept;

	/**
	 * @brief Gets the count of `exception`s in
	 * the stack exception.
	 * 
	 * @return The exception stack size.
	 */
	[[nodiscard]]
	size_t get_exception_count() noexcept;

	/* Debugging */

	/**
	 * @brief Prints a formatted log message with
	 * detailed time information.
	 * 
	 * @param flags Defines how function will behave
	 * and how message will be printed, possible flags are:
	 * `PERS` (persistent), `FATA` (fatal), `ERRO` (error),
	 * `DBUG` (debug), `INFO` (information) and `DEFT` (default).
	 *
	 * @details The message prefix will be printed according
	 * to @p flags parameter. Exceptionally, if `FATA` was wet 
	 * with `ERRO`, the prefix will be `Meg Fatal Error`. `FATA` 
	 * also makes that the log will be printed to `stderr`.
	 * If `PERS` was set, the log message will always be printed,
	 * until with debugger disable.
	 * `DEFT` maskes that message prefix will be only `Meg`.
	 *
	 * @param msg The message to be printed on the stream. Should
	 * not be nullptr.
	 *
	 * @return `true` if the function was succesful; `false` if
	 * not.
	 */
	bool log(uint8_t flags, const char* msg, ...) noexcept;

	/**
	 * @brief Prints a formatted log message with
	 * detailed time information.
	 * 
	 * @param flags Defines how function will behave
	 * and how message will be printed, possible flags are:
	 * `PERS` (persistent), `FATA` (fatal), `ERRO` (error),
	 * `DBUG` (debug), `INFO` (information) and `DEFT` (default).
	 *
	 * @details The message prefix will be printed according
	 * to @p flags parameter. Exceptionally, if `FATA` was wet 
	 * with `ERRO`, the prefix will be `Meg Fatal Error`. `FATA` 
	 * also makes that the log will be printed to `stderr`.
	 * If `PERS` was set, the log message will always be printed,
	 * until with debugger disable.
	 * `DEFT` maskes that message prefix will be only `Meg`.
	 *
	 * @param msg The message to be printed on the stream. Should
	 * not be nullptr.
	 *
	 * @param va A started `va_list` with values to
	 * placeholders in the @p msg string.
	 *
	 * @return `true` if the function was succesful; `false` if
	 * not.
	 */
	bool log(uint8_t flags, const char* msg, va_list va) noexcept;

	/* Printing */

	/**
	 * @brief Prints a message with a prefix based on the
	 * @p flags parameter and formatted with placeholders,
	 * which values are placed in variadic parameters.
	 *
	 * @param flags Defines how function will behave
	 * and how message will be printed, possible flags are:
	 * `FATA` (fatal), `ERRO` (error), `DBUG` (debug),
	 * `INFO` (information) and `DEFT` (default).
	 *
	 * @param msg An string containing the message to be 
	 * printed in the output.
	 *
	 * @warning The @p msg param. Must not be null, if contrary
	 * the function will print "(null)".
	 *
	 * @details The message prefix will be printed according
	 * to @p flags parameter. Exceptionally, if `FATA` was wet 
	 * with `ERRO`, the prefix will be `Meg Fatal Error`. `FATA` 
	 * also makes that the log will be printed to `stderr`.
	 * `DEFT` maskes that message prefix will be only `Meg`. 
	 * this allows you print messages to the output cleanly.
	 *
	 * @return `true` if the function was succesful; and
	 * `false` if not.
	 */
	bool print(uint8_t flags, const char* msg, ...) noexcept;

	/**
	 * @brief Prints a message with a prefix based on the
	 * @p flags parameter and formatted with placeholders,
	 * which values are placed in variadic parameters.
	 *
	 * @param flags Defines how function will behave
	 * and how message will be printed, possible flags are:
	 * `FATA` (fatal), `ERRO` (error), `DBUG` (debug),
	 * `INFO` (information) and `DEFT` (default).
	 *
	 * @param msg An string containing the message to be 
	 * printed in the output.
	 *
	 * @param va A started `va_list` with values to
	 * placeholders in the @p msg string.
	 *
	 * @warning The @p msg param. Must not be null, if contrary
	 * the function will print "(null)".
	 *
	 * @details The message prefix will be printed according
	 * to @p flags parameter. Exceptionally, if `FATA` was wet 
	 * with `ERRO`, the prefix will be `Meg Fatal Error`. `FATA` 
	 * also makes that the log will be printed to `stderr`.
	 * `DEFT` maskes that message prefix will be only `Meg`. 
	 * this allows you print messages to the output cleanly.
	 *
	 * @return `true` if the function was succesful; and
	 * `false` if not.
	 */
	bool print(uint8_t flags, const char* msg, va_list va) noexcept;

	/* Aliases */

	log_alias(err, ERRO);
	log_alias(war, WARN);
	log_alias(deb, DBUG);
	log_alias(inf, INFO);
	log_alias(def, DEFT);

	log_alias(perr, PERS | ERRO);
	log_alias(pwar, PERS | WARN);
	log_alias(pdeb, PERS | DBUG);
	log_alias(pinf, PERS | INFO);
	log_alias(pdef, PERS | DEFT);

	log_alias(ferr, FATA | ERRO);
	log_alias(fwar, FATA | WARN);
	log_alias(fdeb, FATA | DBUG);
	log_alias(finf, FATA | INFO);
	log_alias(fdef, FATA | DEFT);

	log_alias(pferr, PERS | FATA | ERRO);
	log_alias(pfwar, PERS | FATA | WARN);
	log_alias(pfdeb, PERS | FATA | DBUG);
	log_alias(pfinf, PERS | FATA | INFO);
	log_alias(pfdef, PERS | FATA | DEFT);
};	  // namespace dbg
