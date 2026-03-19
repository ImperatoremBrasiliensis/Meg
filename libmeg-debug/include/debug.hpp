#pragma once

#include <common.hpp>

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

const constexpr uint8_t DEFT = 0b00000000;
const constexpr uint8_t ERRO = 0b00000001;
const constexpr uint8_t WARN = 0b00000010;
const constexpr uint8_t DBUG = 0b00000100;
const constexpr uint8_t INFO = 0b00001000;
const constexpr uint8_t FATA = 0b00010000;
const constexpr uint8_t PERS = 0b00100000;

namespace dbg {
	/* Debugger */

	/**
	 * @brief Sets debugging enabled.
	 * 
	 * @details Sets debugger enable or disabled,
	 * which is essential for `dbg::log` function.
	 * call ( @ref log ). If the call to `dbg::log` 
	 * has the `PERS` (persistence) flag, you have
	 * nothing to worry about.
	 *
	 * @param active If it's `true`, debugging will
	 * be enabled; if `false`, debuggig will be 
	 * disabled.
	 *
	 * @return According to @p active parameter, `true` if debugging was
	 * enabled/disabled; `false` if debugging is already
	 * enabled/disabled.
	 */
	bool active_debug(bool active) noexcept;

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
	bool is_debug_active() noexcept;

	/* Exceptions */

	/**
	 * @brief An enum with the all exception ( @ref exception )
	 * codes possible.
	 */
	typedef enum : uint16_t {
		NO_EXCEPTION
	} exception_code;

	/**
	 * @brief Is an struct specifying an exception
	 * and a message explaining it.
	 */
	typedef struct {
		const char* msg;
		exception_code code;
	} exception;

	/** 
	 * @brief Throws an `exception` and stores
	 * it in a stack.
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
	 * @brief Get the `exception` at @p at posistion.
	 * 
	 * @param at The index of the `exception`, beginning
	 * with 0;
	 * 
	 *	@warning The @p at param must be between 0 and
	 * exception count ( @ref get_exception_count() ) - 1.
	 * 
	 * @return an `eception` at @p at ; nothing if an
	 * occurred. 
	 */
	[[nodiscard]]
	dbg::exception get_exception(size_t at) noexcept;

	/**
	 * @brief Get all `exception`s in the stack.
	 * 
	 * @return a $std::vector<std::exception> with
	 * all `exception`s stacked along debug execution.
	 */
	[[nodiscard]]
	std::vector<dbg::exception> get_exceptions() noexcept;

	/**
	 * @brief Get all `exception`s in the stack with the
	 * code (`exception.code`) equal to %code parameter.
	 * 
	 * @param with_code 
	 * @return std::vector<dbg::exception> 
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
	 * @brief Prints a log with detailed date and hour
	 * information and a message with a prefix based on
	 * the @p flags parameter and formatted with placeholders,
	 * which values are placed in @p ... parameter.
	 * 
	 * @param flags Are bitmasks that describes the type
	 * of the message log and that is used to generate 
	 * the prefix. 
	 * @param msg An string containing the message to be 
	 * printed in the output.
	 * @param __VA_ARGS__ Is values to the
	 * placeholders present in the @p msg parameter.
	 *
	 * @warning The @p msg param. must not be null, if contrary
	 * the function will return `false` and will not print 
	 * the log.
	 *
	 * @return `true` if the function was succesful; and
	 * `false` if not.
	 */
	bool log(uint8_t flags, const char* msg, ...) noexcept;

	/**
	 * @brief Prints a log with detailed date and hour
	 * information and a message with a prefix based on
	 * the @p flags parameter and formatted with placeholders,
	 * which values are placed in @p va parameter.
	 * 
	 * @param flags Are bitmasks that describes the type
	 * of the message log and that is used to generate 
	 * the prefix. 
	 * @param msg An string containing the message to be 
	 * printed in the output.
	 * @param va Is values to the placeholders present in
	 * the string in @p msg parameter. This is is a `va_list`
	 * previous started.
	 *
	 * @warning The @p msg param. Must not be null, if contrary
	 * the function will return `false` and will not print 
	 * the log.
	 *
	 * @details In the @p flags parameter, you can pass maskbits
	 * to generate a prefix that can be: `Info`, `Debug`, `Warning`,
	 * `Error`, `Fatal Error` or nothing after "`Meg`". Per example:
	 * @code c++
	 *	dbg::print(FATA | ERRO, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Fatal Error: Your message.`.
	 *	dbg::print(ERRO, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Error: Your message.`.
	 *	dbg::print(WARN, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Warning: Your message.`.
	 *	dbg::print(DBUG, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Debug: Your message.`.
	 *	dbg::print(INFO, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Info: Your message.`.
	 *	dbg::print(DEFT, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg: Your message.`.
	 * @endcode
	 * this allows you print messages to the output cleanly.
	 *
	 * @note The time infomation is display in the 
	 * brasilian format. The date format is `day/month/year`
	 * and the hour is `hour:minutes:seconds` after the `§`
	 * character.
	 *
	 * @details In the @p flags parameter, you can pass maskbits
	 * to generate a prefix that can be: `Info`, `Debug`, `Warning`,
	 * `Error`, `Fatal Error` or nothing after "`Meg`". Per example:
	 * @code c++
	 *	dbg::print(FATA | ERRO, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Fatal Error: Your message.`.
	 *	dbg::print(ERRO, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Error: Your message.`.
	 *	dbg::print(WARN, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Warning: Your message.`.
	 *	dbg::print(DBUG, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Debug: Your message.`.
	 *	dbg::print(INFO, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg Info: Your message.`.
	 *	dbg::print(DEFT, "Your message.\n");
	 * // Output will be similar to: `[dd/mm/yyyy § hh:mm:ss] Meg: Your message.`.
	 * @endcode
	 * this allows you print messages to the output cleanly.
	 *
	 * @return `true` if the function was succesful; and
	 * `false` if not.
	 */
	bool log(uint8_t flags, const char* msg, va_list va) noexcept;

	/* Print */

	/**
	 * @brief Prints a message with a prefix based on the
	 * @p flags parameter and formatted with placeholders,
	 * which values are placed in @p ... parameter.
	 * 
	 * @param flags Are bitmasks that describes the type
	 * of the message log and that is used to generate 
	 * the prefix. 
	 * @param msg An string containing the message to be 
	 * printed in the output.
	 * @param __VA_ARGS__ Is values to the placeholders present in
	 * the string in @p msg parameter. This is is a `va_list`
	 * previous started
	 *
	 * @warning The @p msg param. Must not be null, if contrary
	 * the function will print "(null)".
	 *
	 * @details In the @p flags parameter, you can pass maskbits
	 * to generate a prefix that can be: `Info`, `Debug`, `Warning`,
	 * `Error`, `Fatal Error` or nothing after "`Meg`". Per example:
	 * @code c++
	 *	dbg::print(FATA | ERRO, "Your message.\n");
	 * // Output will be similar to: `Meg Fatal Error: Your message.`.
	 *	dbg::print(ERRO, "Your message.\n");
	 * // Output will be similar to: `Meg Error: Your message.`.
	 *	dbg::print(WARN, "Your message.\n");
	 * // Output will be similar to: `Meg Warning: Your message.`.
	 *	dbg::print(DBUG, "Your message.\n");
	 * // Output will be similar to: `Meg Debug: Your message.`.
	 *	dbg::print(INFO, "Your message.\n");
	 * // Output will be similar to: `Meg Info: Your message.`.
	 *	dbg::print(DEFT, "Your message.\n");
	 * // Output will be similar to: `Meg: Your message.`.
	 * @endcode
	 * this allows you print messages to the output cleanly.
	 *
	 * @return `true` if the function was succesful; and
	 * `false` if not.
	 */
	bool print(uint8_t flags, const char* msg, ...) noexcept;

	/**
	 * @brief Prints a message with a prefix based on the
	 * @p flags parameter and formatted with placeholders,
	 * which values are placed in @p va parameter.
	 *
	 * @param flags Are bitmasks that describes the type
	 * of the message log and that is used to generate 
	 * the prefix.
	 * @param msg An string containing the message to be 
	 * printed in the output.
	 * @param va Is values to the placeholders present
	 * in the @p msg parameter.
	 *
	 * @warning The @p msg param. Must not be null, if contrary
	 * the function will print "(null)".
	 *
	 * @details In the @p flags parameter, you can pass maskbits
	 * to generate a prefix that can be: `Info`, `Debug`, `Warning`,
	 * `Error`, `Fatal Error` or nothing after "`Meg`". Per example:
	 * @code c++
	 *	dbg::print(FATA | ERRO, "Your message.\n");
	 * // Output will be similar to: `Meg Fatal Error: Your message.`.
	 *	dbg::print(ERRO, "Your message.\n");
	 * // Output will be similar to: `Meg Error: Your message.`.
	 *	dbg::print(WARN, "Your message.\n");
	 * // Output will be similar to: `Meg Warning: Your message.`.
	 *	dbg::print(DBUG, "Your message.\n");
	 * // Output will be similar to: `Meg Debug: Your message.`.
	 *	dbg::print(INFO, "Your message.\n");
	 * // Output will be similar to: `Meg Info: Your message.`.
	 *	dbg::print(DEFT, "Your message.\n");
	 * // Output will be similar to: `Meg: Your message.`.
	 * @endcode
	 * this allows you print messages to the output cleanly.
	 *
	 * @return `true` if the function was succesful; and
	 * `false` if not.
	 */
	bool print(uint8_t flags, const char* msg, va_list va) noexcept;

	/* Aliases */

	log_alias(fer, FATA | ERRO);
	log_alias(err, ERRO);
	log_alias(war, WARN);
	log_alias(deb, DBUG);
	log_alias(inf, INFO);
	log_alias(def, DEFT);

	log_alias(pfer, PERS | FATA | ERRO);
	log_alias(perr, PERS | ERRO);
	log_alias(pwar, PERS | WARN);
	log_alias(pdeb, PERS | DBUG);
	log_alias(pinf, PERS | INFO);
	log_alias(pdef, PERS | DEFT);
};	  // namespace dbg
