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
add_subdirectory(tinyobjloader)
add_subdirectory(spdlog)
add_subdirectory(googletest)
add_subdirectory(json)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

include_directories(imgui)
include_directories(imgui/backends)

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
        Async/ThreadPool.h
        Async/ThreadPool.cpp
        Async/threadsafe_queue.h
        Math/GlmHash.h
        )


set(IMGUI
        imgui/backends/imgui_impl_vulkan.h
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/backends/imgui_impl_glfw.h
        imgui/backends/imgui_impl_glfw.cpp
        )

add_library(imgui
        src/stomp_imconfig.h
        imgui/imgui.h
        imgui/imgui.cpp
        imgui/imconfig.h
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_internal.h
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        )
target_compile_definitions(imgui PUBLIC -DIMGUI_USER_CONFIG="stomp_imconfig.h")

list(TRANSFORM SOURCE PREPEND "src/")

include(CMake/SystemLink.cmake)

add_library(stomp_renderer ${SOURCE} ${IMGUI})
target_link_libraries(stomp_renderer PUBLIC renderer::renderer_options renderer::renderer_warnings)
target_link_system_libraries(stomp_renderer PUBLIC Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog nlohmann_json::nlohmann_json)

add_executable(renderer src/main.cpp)
target_link_libraries(renderer stomp_renderer)

include(CTest)
enable_testing()

add_subdirectory(tests)
#TODO CPack
