add_library(meg-common STATIC)

target_sources(
   meg-common
   PUBLIC
   include/internal/common.hpp
   PRIVATE
   src/common.cpp
)

target_include_directories(meg-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)