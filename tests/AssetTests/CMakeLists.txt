
set(TESTS
        InitializationTests.cpp
        AssetSystemTests.cpp
        AssetObjectTest.cpp
        MaterialAssetTest.cpp
        ModelAssetTest.cpp
        SceneAssetTest.cpp
)

add_executable(asset_tests ${TESTS})
target_link_libraries(asset_tests stomp_renderer gtest gtest_main "-static-libgcc -static-libstdc++")
add_test(NAME AssetTests COMMAND asset_tests)
