add_executable(Meg)

target_sources(
   Meg
   PRIVATE
   src/main.cpp
   src/compiler/compiler.cpp
   src/compiler/compiler.hpp
)