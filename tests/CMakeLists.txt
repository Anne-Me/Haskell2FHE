#message(STATUS "TFHE_PREFIX is ${TFHE_PREFIX}")

# Use FetchContent to download gtest
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Make it available to the project
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(unit_tests 
  circuitgraph_tests.cpp
  tfhelib_check.cpp)

# link GoogleTest libraries
# target_link_libraries(unit_tests gtest_main)
#target_include_directories(unit_tests PRIVATE ${TFHE_PREFIX}/include)
#target_link_libraries(unit_tests gtest_main tfhe-spqlios-fma)

target_link_libraries(unit_tests PRIVATE
  gtest_main
  clash2tfhe_lib       
)

# register test with CTest
#add_test(NAME CircuitGraphTests COMMAND unit_tests)

# list all tests
gtest_discover_tests(unit_tests)