cmake_minimum_required(VERSION 3.15)
project(lightning-orderbook VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common flags
add_compile_options(-Wall -Wextra -Wpedantic)

# --- 1. ENGINE CORE (LIST IMPLEMENTATION) ---
file(GLOB_RECURSE ENGINE_CORE_LIST_SOURCES list_impl/src/*.cpp)
add_library(engine_core_list STATIC ${ENGINE_CORE_LIST_SOURCES})
target_include_directories(engine_core_list PUBLIC list_impl/include)

# --- 2. ENGINE CORE (VECTOR IMPLEMENTATION) ---
file(GLOB_RECURSE ENGINE_CORE_VECTOR_SOURCES vector_impl/src/*.cpp)
add_library(engine_core_vector STATIC ${ENGINE_CORE_VECTOR_SOURCES})
target_include_directories(engine_core_vector PUBLIC vector_impl/include)

# --- 3. ITCH PARSER ---
# The parser is implementation-agnostic as types.hpp is identical
file(GLOB_RECURSE ITCH_PARSER_SOURCES itch/src/*.cpp)
add_library(itch_parser STATIC ${ITCH_PARSER_SOURCES})
target_include_directories(itch_parser PUBLIC itch/include)
target_link_libraries(itch_parser PRIVATE engine_core_list)

# --- 4. BENCHMARKS ---
file(GLOB_RECURSE BENCH_SOURCES bench/src/*.cpp)

# Bench for List Implementation
add_executable(bench_list ${BENCH_SOURCES})
target_include_directories(bench_list PRIVATE bench/include list_impl/include)
target_link_libraries(bench_list PRIVATE itch_parser engine_core_list)
target_compile_options(bench_list PRIVATE -O3 -march=native -fno-exceptions -fno-rtti)

# Bench for Vector Implementation
add_executable(bench_vector ${BENCH_SOURCES})
target_include_directories(bench_vector PRIVATE bench/include vector_impl/include)
target_link_libraries(bench_vector PRIVATE itch_parser engine_core_vector)
target_compile_options(bench_vector PRIVATE -O3 -march=native -fno-exceptions -fno-rtti)

# --- 5. TESTS ---
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
add_executable(tests ${TEST_SOURCES} bench/src/latency_stats.cpp bench/src/cycle_clock.cpp)
target_include_directories(tests PRIVATE bench/include list_impl/include)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain itch_parser engine_core_list)
