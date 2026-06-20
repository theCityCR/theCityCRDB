#!/usr/bin/env sh
set -eu

cmake -S . -B build-coverage -DTHECITYCRDB_ENABLE_COVERAGE=ON -DTHECITYCRDB_BUILD_TESTS=ON
cmake --build build-coverage
ctest --test-dir build-coverage --output-on-failure
