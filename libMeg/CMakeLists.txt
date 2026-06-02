list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_library(libMeg SHARED)
set_target_properties(libMeg PROPERTIES OUTPUT_NAME "Meg")

target_sources(
   libMeg
   PUBLIC 
   FILE_SET HEADERS BASE_DIRS include
      FILES
         include/Meg/common.hpp
         include/Meg/filesystem.hpp
         include/Meg/debug.hpp
         include/Meg/lang.hpp
   PRIVATE
   src/debug/debug.cpp
   src/filesystem/path.cpp
   src/common.cpp
   src/lang.cpp
)

set(
   PRE_COMPILED_HEADERS
   <vector>
   <string>
   <cstdio>
   <cstring>
)
target_precompile_headers(libMeg PUBLIC ${PRECOMPILE_HEADERS})

add_subdirectory(unitests)
