/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#pragma once

#include <chrono>
#include <iostream>
#include <source_location>

namespace Meg::dbg {
	/**
    * @brief Prints a log message with time
    * information.
    *
    * **UNIQUE PROPERTY**: Prints all the variadic 
    *                      arguments in order that
    *                      they were puted. As an
    *                      example, the code
    * @code cpp
    * dbg::log(
    *    "Fatal Error",
    *    "Error in ",
    *    18,
    *    " instalations."
    * );
    * @endcode 
    *                      will print something like
    *                      as follows:
    * @code txt
    * (yyyy-mm-dd hh:mm:ss.milescons) $ Meg: Fatal Error: Error in 18 instalations.
    * @endcode
    * 
    * @param prefix The prefix to be puted after `Meg:`
    * in the message.
    * @param va Everything you want to print.
    */
	void log(std::string_view prefix, auto... va) {
		std::cerr
			<< std::format(
				   "\033[1;38;5;008m({:%F %T}) $ \033[0m",
				   std::chrono::system_clock::now()
			   )
			<< "\033[1mMeg\033[0m: " << prefix << ": ";

		(std::cerr << ... << va) << std::endl;
	}

	/*
    * Is only a wrapper to `dbg::log` with the
    * `prefix` parameter defined as 'Info'.
    */
	inline void loginf(auto... msg) {
		log("\033[1;32mInfo\033[0m", msg...);
	}

	/*
    * Is only a wrapper to `dbg::log` with the
    * `prefix` parameter defined as 'Debug'.
    */
	inline void logdbg(auto... msg) {
		log("\033[1mDebug\033[0m", msg...);
	}

	/*
    * Is only a wrapper to `dbg::log` with the
    * `prefix` parameter defined as 'Warning'.
    */
	inline void logwar(auto... msg) {
		log("\033[1;33mWarning\033[0m", msg...);
	}

	/*
    * Is only a wrapper to `dbg::log` with the
    * `prefix` parameter defined as 'Error'.
    */
	inline void logerr(auto... msg) {
		log("\033[1;31mError\033[0m", msg...);
	}

	/**
    * @brief Aborts the program execution with 
    * detailed time and source location 
    * information.
    *
    * **UNIQUE PROPERTY**: Prints a message in the
    *                      `stderr` output with the
    *                      source location (line and 
    *                      column and filename), the
    *                      name of the function that
    *                      pamicked and time information. 
    * 
    * @param msg A simple message to print. If you 
    * prefer, you can format the message before 
    * calling this function.
    * @param loc The source location. By default, is
    * defined as `std::source_location::current()`.
    */
	[[noreturn]]
	void panic(
		std::string_view msg,
		std::source_location loc = std::source_location::current()
	);
}  // namespace Meg::dbg