cmake_minimum_required(VERSION 3.19)
project(renderer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw-3.3.4)
add_subdirectory(glm)
add_subdirectory(tinyobjloader-master)
add_subdirectory(spdlog)
add_subdirectory(googletest)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

include_directories(imgui-master)
include_directories(imgui-master/backends)

set(SOURCE
        Renderer.cpp
        Renderer.h
        Model.h
        Model.cpp
        Scene.h
        Scene.cpp
        stb_image.h
        stb_image.cpp
        tinyobjloader.cpp
        )

set(TESTS
        InitializationTests.cpp
        )

set(IMGUI
        imgui-master/backends/imgui_impl_vulkan.h
        imgui-master/backends/imgui_impl_vulkan.cpp
        imgui-master/backends/imgui_impl_glfw.h
        imgui-master/backends/imgui_impl_glfw.cpp
        )

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

list(TRANSFORM TESTS PREPEND "tests/")
list(TRANSFORM SOURCE PREPEND "src/")

enable_testing()

add_executable(renderer src/main.cpp ${SOURCE} ${IMGUI})
target_compile_definitions(renderer PRIVATE)
target_link_libraries(renderer Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog)

add_executable(tests ${TESTS} ${SOURCE} ${IMGUI})
add_test(NAME RendererTests COMMAND tests)
target_compile_definitions(tests PRIVATE)
target_link_libraries(tests gtest gtest_main imgui Vulkan::Vulkan glfw glm::glm tinyobjloader imgui spdlog::spdlog)
