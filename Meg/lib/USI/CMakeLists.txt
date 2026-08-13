add_library(libMeg-USI OBJECT)
set_target_properties(libMeg-USI PROPERTIES OUTPUT_NAME "Meg-USI")

target_sources(
   libMeg-USI
   PUBLIC
   FILE_SET HEADERS BASE_DIRS include
      FILES
      include/USI/filesystem.hpp
   PRIVATE
   src/filesystem.cpp
)

target_link_libraries(libMeg-USI PUBLIC libMeg-Debug)

add_subdirectory(unitests)
