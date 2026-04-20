cmake_minimum_required(VERSION 4.2.0) 
project(
   Meg VERSION 0.1.0 
   LANGUAGES C CXX
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   add_compile_definitions(DEBUG)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_C_EXTENSIONS off)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)
set(CXX_SCAN_FOR_MODULES off)

# Compiler
add_subdirectory(liblunique)
add_subdirectory(Meg)