set(TESTS
	CoreTest.cpp
)


add_executable(core_tests ${TESTS})
target_link_libraries(core_tests stomp_renderer gtest gtest_main "-static-libgcc -static-libstdc++")
add_test(NAME CoreTests COMMAND core_tests)
