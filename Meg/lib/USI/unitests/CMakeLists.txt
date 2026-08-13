include(MegTestRules)

add_subdirectory(Filesystem)

Meg_add_test(FileSystemTest SRC FileSystemTest.cpp)
