cmake_minimum_required(VERSION 3.15)
project(lightning-orderbook VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common flags
add_compile_options(-Wall -Wextra -Wpedantic)

# 1. engine_core - static library
file(GLOB_RECURSE ENGINE_CORE_SOURCES src/*.cpp)
add_library(engine_core STATIC ${ENGINE_CORE_SOURCES})
target_include_directories(engine_core PUBLIC include)

# 2. itch_parser - static library
file(GLOB_RECURSE ITCH_PARSER_SOURCES itch/src/*.cpp)
add_library(itch_parser STATIC ${ITCH_PARSER_SOURCES})
target_include_directories(itch_parser PUBLIC itch/include)
target_link_libraries(itch_parser PRIVATE engine_core)

# 3. bench - executable
file(GLOB_RECURSE BENCH_SOURCES bench/src/*.cpp)
add_executable(bench ${BENCH_SOURCES})
target_include_directories(bench PRIVATE bench/include)
target_link_libraries(bench PRIVATE itch_parser engine_core)

# Performance flags for bench
target_compile_options(bench PRIVATE -O3 -march=native -fno-exceptions -fno-rtti)

# 4. tests - executable using Catch2
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
add_executable(tests ${TEST_SOURCES} bench/src/latency_stats.cpp)
target_include_directories(tests PRIVATE bench/include)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain itch_parser engine_core)
