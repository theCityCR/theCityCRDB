cmake_minimum_required(VERSION 3.24)

project(
    theCityCRDB
    VERSION 0.1.0
    DESCRIPTION "A high-performance in-memory relational database engine"
    LANGUAGES CXX
)

option(THECITYCRDB_BUILD_TESTS "Build theCityCRDB tests" ON)
option(THECITYCRDB_BUILD_BENCHMARKS "Build theCityCRDB benchmarks" OFF)
option(THECITYCRDB_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(THECITYCRDB_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(THECITYCRDB_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)
option(THECITYCRDB_ENABLE_COVERAGE "Enable compiler coverage instrumentation" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
include(GNUInstallDirs)

add_library(theCityCRDB
    src/common/value.cpp
    src/parser/parser.cpp
    src/parser/tokenizer.cpp
    src/storage/database.cpp
    src/storage/buffer_pool.cpp
    src/storage/table.cpp
    src/indexing/btree_index.cpp
    src/indexing/hash_index.cpp
    src/execution/query_executor.cpp
    src/planner/query_planner.cpp
    src/concurrency/lock_manager.cpp
    src/persistence/storage_manager.cpp
    src/persistence/write_ahead_log.cpp
    src/transaction/mvcc_row_store.cpp
    src/transaction/transaction_manager.cpp
)

add_library(theCityCRDB::theCityCRDB ALIAS theCityCRDB)

target_include_directories(theCityCRDB
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(theCityCRDB PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(theCityCRDB PRIVATE /W4 /permissive-)
    if(THECITYCRDB_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(theCityCRDB PRIVATE /WX)
    endif()
else()
    target_compile_options(theCityCRDB PRIVATE -Wall -Wextra -Wpedantic)
    if(THECITYCRDB_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(theCityCRDB PRIVATE -Werror)
    endif()
endif()

if(THECITYCRDB_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(theCityCRDB PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(theCityCRDB PUBLIC -fsanitize=address,undefined)
endif()

if(THECITYCRDB_ENABLE_COVERAGE AND NOT MSVC)
    target_compile_options(theCityCRDB PUBLIC --coverage -O0 -g)
    target_link_options(theCityCRDB PUBLIC --coverage)
endif()

if(THECITYCRDB_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    set_target_properties(theCityCRDB PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

add_executable(theCityCRDB_cli src/main.cpp)
target_link_libraries(theCityCRDB_cli PRIVATE theCityCRDB::theCityCRDB)

enable_testing()

if(THECITYCRDB_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(theCityCRDB_tests
        tests/parser_tests.cpp
        tests/storage_tests.cpp
        tests/index_tests.cpp
        tests/execution_tests.cpp
        tests/deep_feature_tests.cpp
    )
    target_link_libraries(theCityCRDB_tests PRIVATE theCityCRDB::theCityCRDB GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(theCityCRDB_tests)
endif()

if(THECITYCRDB_BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)

    add_executable(theCityCRDB_benchmarks benchmarks/storage_benchmarks.cpp)
    target_link_libraries(theCityCRDB_benchmarks PRIVATE theCityCRDB::theCityCRDB benchmark::benchmark_main)
endif()
