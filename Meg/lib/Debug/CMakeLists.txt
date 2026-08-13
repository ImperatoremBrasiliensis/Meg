add_library(libMeg-Debug OBJECT)
set_target_properties(libMeg-Debug PROPERTIES OUTPUT_NAME "Meg-Debug")

target_sources(
   libMeg-Debug
   PUBLIC
   FILE_SET HEADERS BASE_DIRS include
      FILES 
      include/Debug/debug.hpp
   PRIVATE
   src/debug.cpp
)