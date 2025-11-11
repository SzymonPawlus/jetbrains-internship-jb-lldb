# Minimum test
add_executable(basic_test tested_programs/basic_test.cpp)
set_target_properties(basic_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tested_programs)

target_compile_options(basic_test PRIVATE -O0)

# Minimum no-pie test
add_executable(basic_no_pie_test tested_programs/basic_no_pie_test.cpp)
set_target_properties(basic_no_pie_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tested_programs)

target_compile_options(basic_no_pie_test PRIVATE -no-pie -O0)
target_link_options(basic_no_pie_test PRIVATE -no-pie)


# Generate an invalid ELF file for testing purposes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tested_programs/invalid_file
    COMMAND ${CMAKE_COMMAND} -E echo "this is not an ELF, but it should be long enought that it vanishes on validation and not on the loading stage when it finds out it is too short" > ${CMAKE_CURRENT_BINARY_DIR}/tested_programs/invalid_file
    COMMENT "Generating invalid ELF file for tests"
)

add_custom_target(generate_invalid_file ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tested_programs/invalid_file
)

# Make test executable depend on it

add_executable(tests test_elf.cpp test_process.cpp)
target_link_libraries(tests PRIVATE gwatch_lib GTest::gtest_main)
add_dependencies(tests generate_invalid_file)

include(GoogleTest)
gtest_discover_tests(tests)
