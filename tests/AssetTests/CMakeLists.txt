
set(TESTS
        InitializationTests.cpp
        AssetSystemTests.cpp
        MaterialAssetTest.cpp
        ModelAssetTest.cpp
        SceneAssetTest.cpp
)


add_executable(asset_tests ${TESTS})
target_link_libraries(asset_tests stomp_renderer gtest gtest_main Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog nlohmann_json::nlohmann_json renderer::renderer_options renderer::renderer_warnings)
add_test(NAME AssetTests COMMAND asset_tests)
