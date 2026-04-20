add_library(lunique SHARED)

target_sources(
   lunique
   PUBLIC 
   FILE_SET HEADERS BASE_DIRS include
      FILES
      include/lunique/common.hpp
      include/lunique/filesystem.hpp
      include/lunique/debug.hpp
      include/lunique/lang.hpp
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

target_precompile_headers(lunique PUBLIC ${PRECOMPILE_HEADERS})