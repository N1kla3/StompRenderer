cmake_minimum_required(VERSION 3.19)
project(renderer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw-3.3.4)
add_subdirectory(glm)
add_subdirectory(tinyobjloader-master)

add_executable(renderer main.cpp Application.cpp Application.h stb_image.h stb_image.cpp tinyobjloader.cpp)
target_compile_definitions(renderer PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_link_libraries(renderer Vulkan::Vulkan glfw glm::glm tinyobjloader)