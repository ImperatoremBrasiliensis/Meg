# Meg Project Note

This directory contains a minimal, customized integration of GoogleTest (GTest)
v1.17.0, the Google C++ Testing Framework.

## Overview

GoogleTest is a unit testing library for C++ that provides a rich set of
assertions, death tests, parameterized tests, and more. This integration focuses
on the core testing functionality without additional components like samples or
extensive documentation.

## Changes Made

- Based on GoogleTest v1.17.0 from the official GitHub repository.
- Removed unnecessary directories: `docs/`, `samples/`, `test/`.
- Removed `src/gtest_main.cc` to eliminate built-in main function support.
- Removed all `.md` files except `LICENSE`.
- Removed all `.txt` files except `CMakeLists.txt`.
- Kept only essential source files, headers, and CMake configuration.
- Fixed the functions `PrintTo(char8_t c, ::std::ostream* os)` and
  `PrintTo(char8_t c, ::std::ostream* os)` to work around the [Issue
  #4762](https://github.com/google/googletest) in
  include/gtest/gtest-printers.h.

## License

See the `LICENSE` file for the BSD 3-Clause terms.
