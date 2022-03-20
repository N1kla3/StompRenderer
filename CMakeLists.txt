cmake_minimum_required(VERSION 3.19)
project(renderer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw-3.3.4)
add_subdirectory(glm)
add_subdirectory(tinyobjloader-master)
add_subdirectory(spdlog)

include_directories(imgui-master)
include_directories(imgui-master/backends)

add_library(imgui
        imgui-master/imgui.h
        imgui-master/imgui.cpp
        imgui-master/imconfig.h
        imgui-master/imgui_demo.cpp
        imgui-master/imgui_draw.cpp
        imgui-master/imgui_internal.h
        imgui-master/imgui_tables.cpp
        imgui-master/imgui_widgets.cpp
        imgui-master/imstb_rectpack.h
        imgui-master/imstb_textedit.h
        imgui-master/imstb_truetype.h
        )

add_executable(renderer main.cpp Renderer.cpp Renderer.h stb_image.h stb_image.cpp tinyobjloader.cpp
        imgui-master/backends/imgui_impl_vulkan.h
        imgui-master/backends/imgui_impl_vulkan.cpp
        imgui-master/backends/imgui_impl_glfw.h
        imgui-master/backends/imgui_impl_glfw.cpp
        )
target_compile_definitions(renderer PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_link_libraries(renderer Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog)