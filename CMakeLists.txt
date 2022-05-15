cmake_minimum_required(VERSION 3.19)
project(renderer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw-3.3.4)
add_subdirectory(glm)
add_subdirectory(tinyobjloader-master)
add_subdirectory(spdlog)
add_subdirectory(googletest)
add_subdirectory(json)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

include_directories(imgui-docking)
include_directories(imgui-docking/backends)

set(SOURCE
        Renderer.cpp
        Renderer.h
        Model.h
        Model.cpp
        Scene.h
        Scene.cpp
        Texture.h
        Texture.cpp
        MaterialManager.h
        MaterialManager.cpp
        Material.h
        Material.cpp
        VulkanHelper.h
        VulkanHelper.cpp
        Shader.h
        Shader.cpp
        FileDialog.h
        FileDialog.cpp
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
        stb_image.h
        stb_image.cpp
        tinyobjloader.cpp
        Logs.h
        Logs.cpp
        )

set(TESTS
        InitializationTests.cpp
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

enable_testing()

add_executable(renderer src/main.cpp ${SOURCE} ${IMGUI})
target_compile_definitions(renderer PRIVATE)
target_link_libraries(renderer Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog nlohmann_json::nlohmann_json)

add_executable(tests ${TESTS} ${SOURCE} ${IMGUI})
add_test(NAME RendererTests COMMAND tests)
target_compile_definitions(tests PRIVATE)
target_link_libraries(tests gtest gtest_main Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog nlohmann_json::nlohmann_json)
