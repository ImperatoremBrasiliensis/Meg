list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_library(libprotocols SHARED)
set_target_properties(libprotocols PROPERTIES OUTPUT_NAME "protocols")

target_sources(
   libprotocols
   PUBLIC
   FILE_SET HEADERS BASE_DIRS include 
      FILES
         include/protocols/main.h
         include/protocols/Event.h
         include/protocols/Utilities.h
   PRIVATE
   include/internal/main.h
   include/internal/Utilities.h
   include/internal/code/Bytecode.h
   include/internal/code/Source.h
   src/main.c
   src/Utilities.c
)

add_subdirectory(unitests)
