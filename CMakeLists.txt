cmake_minimum_required(VERSION 3.31.0)
project(
   Meg VERSION 0.1.0 
   LANGUAGES C CXX
)

add_compile_options(-Wall -Wextra -Werror)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_C_EXTENSIONS off)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

add_subdirectory(Meg)