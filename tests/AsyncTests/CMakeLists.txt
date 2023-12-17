
set(TESTS
        ThreadPoolTests.cpp
)


add_executable(async_tests ${TESTS})
target_link_libraries(async_tests stomp_renderer gtest gtest_main)
add_test(NAME AsyncTests COMMAND async_tests)
