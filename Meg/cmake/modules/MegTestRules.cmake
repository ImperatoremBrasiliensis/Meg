function(Meg_add_test test_name)
   cmake_parse_arguments(ARG "DEPENDS" "" "SRC" ${ARGN})

   if(NOT ARG_SRC)
      message(FATAL_ERROR "'Meg_add_test()' requires a list of .cpp files "
                          "in parameter SRC")
   endif()

   if(ARG_DEPENDS)
      set_tests_properties(${test_name} PROPERTIES DEPENDS ARG_DEPENDS)
   endif()

   add_executable(${test_name} ${ARG_SRC})
   target_compile_options(${test_name} PUBLIC -Wcharacter-conversion)
   target_link_libraries(${test_name} PUBLIC libMeg gtest)

   include(GoogleTest)
   gtest_discover_tests(${test_name})
endfunction(Meg_add_test)