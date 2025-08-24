message(STATUS "TFHE_PREFIX is ${TFHE_PREFIX}")

# Use FetchContent to download gtest
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Make it available to the project
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(unit_tests circuitgraph_tests.cpp)

# Link GoogleTest libraries
# target_link_libraries(unit_tests gtest_main)
target_include_directories(unit_tests PRIVATE ${TFHE_PREFIX}/include)
target_link_libraries(unit_tests gtest_main tfhe-spqlios-fma)


# Register test with CTest
add_test(NAME CircuitGraphTests COMMAND unit_tests)