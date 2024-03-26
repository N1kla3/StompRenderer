
set(TESTS
        ThreadPoolTests.cpp
        SafeQueueTests.cpp
        JsonAsyncTests.cpp
        SafeMapTests.cpp
)


add_executable(async_tests ${TESTS})
target_link_libraries(async_tests stomp_renderer gtest gtest_main "-static-libgcc -static-libstdc++")
add_test(NAME AsyncTests COMMAND async_tests)
