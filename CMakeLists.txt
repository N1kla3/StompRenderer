cmake_minimum_required(VERSION 3.21)
project(renderer)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_EXTENSIONS OFF)

include(ProjectOptions.cmake)
renderer_setup_options()
renderer_local_options()

add_library(renderer::renderer_options ALIAS renderer_options)
add_library(renderer::renderer_warnings ALIAS renderer_warnings)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(tinyobjloader-master)
add_subdirectory(spdlog)
add_subdirectory(googletest)
add_subdirectory(json)
add_subdirectory(profiler)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

include_directories(imgui-docking)
include_directories(imgui-docking/backends)

set(SOURCE
        Renderer.cpp
        Renderer.h
        Rendering/Model.h
        Rendering/Model.cpp
        ModelAsset.h
        ModelAsset.cpp
        Rendering/ModelStatics.cpp
        Rendering/ModelStatics.h
        Scene.h
        Scene.cpp
        SceneEntity.h
        SceneEntity.cpp
        SceneAsset.h
        SceneAsset.cpp
        Rendering/Texture.h
        Rendering/Texture.cpp
        MaterialManager.h
        MaterialManager.cpp
        Rendering/Material.h
        Rendering/Material.cpp
        MaterialInstance.h
        MaterialInstance.cpp
        MaterialAsset.h
        MaterialAsset.cpp
        Rendering/VulkanContext.h
        Rendering/VulkanContext.cpp
        Rendering/Shader.h
        Rendering/Shader.cpp
        FileDialog.h
        FileDialog.cpp
        ISaveable.h
        Asset.h
        Asset.cpp
        Camera.h
        Camera.cpp
        AssetManager.h
        AssetManager.cpp
        Light.h
        Light.cpp
        LightObject.h
        LightSystem.h
        LightSystem.cpp
        Rendering/GraphicsPipeline.h
        Rendering/GraphicsPipeline.cpp
        AssetLoader.h
        AssetLoader.cpp
        UI/ImguiUnit.h
        UI/ImguiUnit.cpp
        UI/ViewPort.cpp
        UI/ViewPort.h
        UI/MainLayer.h
        UI/MainLayer.cpp
        UI/ScenePanel.cpp
        UI/ScenePanel.h
        UI/EntityPanel.cpp
        UI/EntityPanel.h
        UI/ContentBrowser.h
        UI/ContentBrowser.cpp
        UI/MaterialPanel.h
        UI/MaterialPanel.cpp
        UI/AssetRepresentation.h
        UI/ModelRepresentation.h
        UI/ModelRepresentation.cpp
        UI/MaterialRepresentation.h
        UI/MaterialRepresentation.cpp
        UI/GlobalLightPanel.cpp
        UI/GlobalLightPanel.h
        UI/CameraPanel.h
        UI/CameraPanel.cpp
        UI/IDrawable.h
        stb_image.h
        stb_image.cpp
        tinyobjloader.cpp
        Logs.h
        Logs.cpp
        Rendering/FrameBuffer.h
        Rendering/FrameBuffer.cpp
        Rendering/RenderPass.h
        Rendering/RenderPass.cpp
        Rendering/UniformBuffer.h
        Rendering/UniformBuffer.cpp
        Rendering/ModelInstance.h
        Rendering/ModelInstance.cpp
        ImGuizmo/ImGuizmo.cpp
        ImGuizmo/ImGuizmo.h
        )

set(TESTS
        InitializationTests.cpp
        AssetSystemTests.cpp
        MaterialAssetTest.cpp
        ModelAssetTest.cpp
        SceneAssetTest.cpp
        )

set(IMGUI
        imgui-docking/backends/imgui_impl_vulkan.h
        imgui-docking/backends/imgui_impl_vulkan.cpp
        imgui-docking/backends/imgui_impl_glfw.h
        imgui-docking/backends/imgui_impl_glfw.cpp
        )

add_library(imgui
        imgui-docking/imgui.h
        imgui-docking/imgui.cpp
        imgui-docking/imconfig.h
        imgui-docking/imgui_demo.cpp
        imgui-docking/imgui_draw.cpp
        imgui-docking/imgui_internal.h
        imgui-docking/imgui_tables.cpp
        imgui-docking/imgui_widgets.cpp
        imgui-docking/imstb_rectpack.h
        imgui-docking/imstb_textedit.h
        imgui-docking/imstb_truetype.h
        )

list(TRANSFORM TESTS PREPEND "tests/")
list(TRANSFORM SOURCE PREPEND "src/")

add_executable(renderer src/main.cpp ${SOURCE} ${IMGUI})
target_link_libraries(renderer Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog nlohmann_json::nlohmann_json stompprofiler renderer::renderer_options renderer::renderer_warnings)

# TODO make subdirs for tests

include(CTest)

add_executable(tests ${TESTS} ${SOURCE} ${IMGUI})
target_link_libraries(tests gtest gtest_main Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog nlohmann_json::nlohmann_json renderer::renderer_options renderer::renderer_warnings)

enable_testing()
add_test(NAME RendererTests COMMAND tests)

#TODO CPack
