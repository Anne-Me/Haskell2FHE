
cmake_minimum_required(VERSION 3.5)
project(clash2tfhe)

# Allow overriding TFHE installation prefix; defaults to /usr/local
if(NOT DEFINED TFHE_PREFIX)
    set(TFHE_PREFIX "/usr/local" CACHE PATH "Prefix for TFHE installation")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# Only add /permissive- if MSVC is the compiler
if (MSVC)
    target_compile_options(clash2tfhe PRIVATE /permissive-)
endif()

# Include and link directories for TFHE
#include_directories("${TFHE_PREFIX}/include")
link_directories("${TFHE_PREFIX}/lib")

#find_package(nlohmann_json 3.2.0 REQUIRED)
#if (NOT DEFINED NLOHMANN_JSON_DIR)
#    set(NLOHMANN_JSON_DIR CMAKE_CURRENT_LIST_DIR)
#endif()


set(SOURCES
    main.cpp
    Evaluator.cpp
    CircuitGraph.cpp
    tests/circuitgraph_tests.cpp
)

add_executable(clash2tfhe ${SOURCES})
target_include_directories(clash2tfhe 
    PRIVATE 
    "${NLOHMANN_JSON_DIR}"
    "${CMAKE_SOURCE_DIR}/nlohmann"
)

target_link_libraries(clash2tfhe PRIVATE tfhe-spqlios-fma)

# TFHE TOOLS
add_executable(keygen tfhe-tools/keygen.c)
add_executable(encrypt tfhe-tools/encrypt.c)
add_executable(decrypt tfhe-tools/decrypt.c)

target_include_directories(keygen PRIVATE ${TFHE_PREFIX}/include)
target_include_directories(encrypt PRIVATE ${TFHE_PREFIX}/include)
target_include_directories(decrypt PRIVATE ${TFHE_PREFIX}/include)

target_link_libraries(keygen tfhe-spqlios-fma)
target_link_libraries(encrypt tfhe-spqlios-fma)
target_link_libraries(decrypt tfhe-spqlios-fma)