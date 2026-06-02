function(libprotocols_add_test test_name)
   cmake_parse_arguments(ARG "DEPENDS" "" "SRC" ${ARGN})

   if(NOT ARG_SRC)
      message(FATAL_ERROR "'libprotocols_add_test()' requires a list of .cpp files "
                          "on argument SRC")
   endif()

   if(ARG_DEPENDS)
      set_tests_properties(${test_name} PROPERTIES DEPENDS ARG_DEPENDS)
   endif()

   add_executable(${test_name} ${ARG_SRC})
   target_link_libraries(${test_name} PUBLIC libprotocols gtest)

   include(GoogleTest)
   gtest_discover_tests(${test_name})
endfunction(libprotocols_add_test)
