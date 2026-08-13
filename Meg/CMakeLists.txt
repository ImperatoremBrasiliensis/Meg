list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_subdirectory(lib)

add_library(
   libMeg SHARED
   $<TARGET_OBJECTS:libMeg-USI>
   $<TARGET_OBJECTS:libMeg-Debug>
)
set_target_properties(libMeg PROPERTIES OUTPUT_NAME "Meg")

target_link_libraries(
   libMeg PUBLIC
   libMeg-Debug
   libMeg-USI
)

