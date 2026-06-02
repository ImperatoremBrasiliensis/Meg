cmake_minimum_required(VERSION 3.20.0) 
project(
   Meg VERSION 0.1.0 
   LANGUAGES C CXX
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   add_compile_definitions(DEBUG)
endif()

if (MEG_ENABLE_TESTING)
   enable_testing()
   message("Tests are enabled.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_C_EXTENSIONS off)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

add_library(itfc-pch_headers INTERFACE)
target_precompile_headers(
   itfc-pch_headers INTERFACE 
   <gtest/gtest.h> 
   <gtest/gtest_pred_impl.h>
   <gtest/gtest_prod.h>
   <gtest/gtest-assertion-result.h>
   <gtest/gtest-matchers.h>
   <gtest/gtest-message.h>
   <gtest/gtest-printers.h>
   <gtest/gtest-spi.h>
   <gtest/gtest-test-part.h>
   <gtest/gtest-typed-test.h>
)

# Third-party libraries.
add_subdirectory(third-party)

# protocols
add_subdirectory(libprotocols)

# Meg compiler
add_subdirectory(libMeg)
#add_subdirectory(Meg)
