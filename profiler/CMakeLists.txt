cmake_minimum_required(VERSION 3.19)
project(stompprofiler)

set(CMAKE_CXX_STANDARD 17)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(src)

set(SOURCE
        ProfilerCore.h
        ProfilerCore.cpp
        )

set(TESTS
        InitializationTests.cpp
        )

list(TRANSFORM TESTS PREPEND "tests/")
list(TRANSFORM SOURCE PREPEND "src/")

add_library(stompprofiler ${SOURCE})
target_compile_definitions(stompprofiler PRIVATE)
target_link_libraries(stompprofiler nlohmann_json::nlohmann_json)

add_executable(profiler_tests ${TESTS} ${SOURCE})
add_test(NAME ProfilerTests COMMAND tests)
target_compile_definitions(profiler_tests PRIVATE)
target_link_libraries(profiler_tests gtest gtest_main nlohmann_json::nlohmann_json)
