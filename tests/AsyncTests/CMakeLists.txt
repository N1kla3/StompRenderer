
set(TESTS
        ThreadPoolTests.cpp
        SafeQueueTests.cpp
        JsonAsyncTests.cpp
        SafeMapTests.cpp
)


add_executable(async_tests ${TESTS})
if (MINGW)
  target_link_libraries(async_tests stomp_renderer gtest gtest_main "-static-libgcc -static-libstdc++")
else()
  target_link_libraries(async_tests stomp_renderer gtest gtest_main)
endif()

add_test(NAME AsyncTests COMMAND async_tests)

