cmake_minimum_required(VERSION 3.24)

project(
    AtlasDB
    VERSION 0.1.0
    DESCRIPTION "A high-performance in-memory relational database engine"
    LANGUAGES CXX
)

option(ATLASDB_BUILD_TESTS "Build AtlasDB tests" ON)
option(ATLASDB_BUILD_BENCHMARKS "Build AtlasDB benchmarks" OFF)
option(ATLASDB_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ATLASDB_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(ATLASDB_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
include(GNUInstallDirs)

add_library(atlasdb
    src/common/value.cpp
    src/parser/parser.cpp
    src/parser/tokenizer.cpp
    src/storage/database.cpp
    src/storage/table.cpp
    src/indexing/hash_index.cpp
    src/execution/query_executor.cpp
    src/concurrency/lock_manager.cpp
    src/persistence/storage_manager.cpp
)

add_library(AtlasDB::atlasdb ALIAS atlasdb)

target_include_directories(atlasdb
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(atlasdb PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(atlasdb PRIVATE /W4 /permissive-)
    if(ATLASDB_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(atlasdb PRIVATE /WX)
    endif()
else()
    target_compile_options(atlasdb PRIVATE -Wall -Wextra -Wpedantic)
    if(ATLASDB_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(atlasdb PRIVATE -Werror)
    endif()
endif()

if(ATLASDB_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(atlasdb PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(atlasdb PRIVATE -fsanitize=address,undefined)
endif()

if(ATLASDB_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    set_target_properties(atlasdb PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

add_executable(atlasdb_cli src/main.cpp)
target_link_libraries(atlasdb_cli PRIVATE AtlasDB::atlasdb)

enable_testing()

if(ATLASDB_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(atlasdb_tests
        tests/parser_tests.cpp
        tests/storage_tests.cpp
        tests/index_tests.cpp
        tests/execution_tests.cpp
    )
    target_link_libraries(atlasdb_tests PRIVATE AtlasDB::atlasdb GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(atlasdb_tests)
endif()

if(ATLASDB_BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)

    add_executable(atlasdb_benchmarks benchmarks/storage_benchmarks.cpp)
    target_link_libraries(atlasdb_benchmarks PRIVATE AtlasDB::atlasdb benchmark::benchmark_main)
endif()
