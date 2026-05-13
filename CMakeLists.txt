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

# --- 2b. ENGINE CORE (F_VECTOR IMPLEMENTATION) ---
file(GLOB_RECURSE ENGINE_CORE_F_VECTOR_SOURCES f_vector_impl/src/*.cpp)
add_library(engine_core_f_vector STATIC ${ENGINE_CORE_F_VECTOR_SOURCES})
target_include_directories(engine_core_f_vector PUBLIC f_vector_impl/include)

# --- 3. ITCH PARSER ---
file(GLOB_RECURSE ITCH_PARSER_SOURCES itch/src/*.cpp)

# We create three parser libraries, one for each implementation
add_library(itch_parser_list STATIC ${ITCH_PARSER_SOURCES})
target_include_directories(itch_parser_list PUBLIC itch/include)
target_link_libraries(itch_parser_list PRIVATE engine_core_list)

add_library(itch_parser_vector STATIC ${ITCH_PARSER_SOURCES})
target_include_directories(itch_parser_vector PUBLIC itch/include)
target_link_libraries(itch_parser_vector PRIVATE engine_core_vector)

add_library(itch_parser_f_vector STATIC ${ITCH_PARSER_SOURCES})
target_include_directories(itch_parser_f_vector PUBLIC itch/include)
target_link_libraries(itch_parser_f_vector PRIVATE engine_core_f_vector)

# --- 4. BENCHMARKS ---
file(GLOB_RECURSE BENCH_SOURCES bench/src/*.cpp)

# Bench for List Implementation
add_executable(bench_list ${BENCH_SOURCES})
target_include_directories(bench_list PRIVATE bench/include list_impl/include)
target_link_libraries(bench_list PRIVATE itch_parser_list engine_core_list)
target_compile_options(bench_list PRIVATE -O3 -march=native -fno-exceptions -fno-rtti)

# Bench for Vector Implementation
add_executable(bench_vector ${BENCH_SOURCES})
target_include_directories(bench_vector PRIVATE bench/include vector_impl/include)
target_link_libraries(bench_vector PRIVATE itch_parser_vector engine_core_vector)
target_compile_options(bench_vector PRIVATE -O3 -march=native -fno-exceptions -fno-rtti)

# Bench for F_Vector Implementation
add_executable(bench_f_vector ${BENCH_SOURCES})
target_include_directories(bench_f_vector PRIVATE bench/include f_vector_impl/include)
target_link_libraries(bench_f_vector PRIVATE itch_parser_f_vector engine_core_f_vector)
target_compile_options(bench_f_vector PRIVATE -O3 -march=native -fno-exceptions -fno-rtti)

# --- 5. TESTS ---
enable_testing()
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Common test sources and support files
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
set(TEST_SUPPORT_SOURCES bench/src/latency_stats.cpp bench/src/cycle_clock.cpp)

# Helper function to create test targets
function(add_engine_test target_name core_lib parser_lib include_dir)
    add_executable(${target_name} ${TEST_SOURCES} ${TEST_SUPPORT_SOURCES})
    target_include_directories(${target_name} PRIVATE itch/include bench/include ${include_dir})
    target_link_libraries(${target_name} PRIVATE Catch2::Catch2WithMain ${parser_lib} ${core_lib})
    
    include(Catch)
    catch_discover_tests(${target_name})
endfunction()

# Test List Implementation
add_engine_test(test_list engine_core_list itch_parser_list list_impl/include)

# Test Vector Implementation
add_engine_test(test_vector engine_core_vector itch_parser_vector vector_impl/include)

# Test F_Vector Implementation
add_engine_test(test_f_vector engine_core_f_vector itch_parser_f_vector f_vector_impl/include)
