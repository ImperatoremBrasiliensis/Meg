add_library(meg-debug SHARED)

target_sources(
   meg-debug
   PUBLIC
   include/debug.hpp
   PRIVATE
   src/debug.cpp
)

target_include_directories(meg-debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(meg-debug PUBLIC meg-common)