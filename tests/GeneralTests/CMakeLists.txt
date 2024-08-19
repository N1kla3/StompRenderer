set(TESTS
	CoreTest.cpp
)


add_executable(core_tests ${TESTS})
if (MINGW)
  target_link_libraries(core_tests stomp_renderer gtest gtest_main "-static-libgcc -static-libstdc++")
else()
  target_link_libraries(core_tests stomp_renderer gtest gtest_main)
endif()

add_test(NAME CoreTests COMMAND core_tests)

