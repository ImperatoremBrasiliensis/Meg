# Meg Project Note

This directory contains a minimal, customized integration of GoogleMock (GMock) v1.17.0, the Google C++ Mocking Framework.

## Overview

GoogleMock is a library for creating mock objects in C++ tests. It integrates seamlessly with GoogleTest and provides powerful mocking capabilities including expectations, matchers, and actions.

## Changes Made

- Based on GoogleMock v1.17.0 from the official GoogleTest repository.
- Removed unnecessary directories: `docs/`, `test/`.
- Removed `src/gmock_main.cc` to eliminate built-in main function support.
- Removed all `.md` files except `LICENSE`.
- Removed all `.txt` files except `CMakeLists.txt`.
- Kept only essential source files, headers, and CMake configuration.

## License

See the `LICENSE` file for the BSD 3-Clause terms.
