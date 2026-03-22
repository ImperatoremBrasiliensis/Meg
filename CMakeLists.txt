cmake_minimum_required(VERSION 4.2.0) 
project(
   Meg VERSION 0.1.0 
   LANGUAGES C CXX
)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   #add_compile_definitions(DEBUG)
endif()

add_compile_options(-Wall -Wextra -Werror -pedantic-errors)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_C_EXTENSIONS off)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

add_subdirectory(libmeg-common)
add_subdirectory(libmeg-debug)
add_subdirectory(Meg)