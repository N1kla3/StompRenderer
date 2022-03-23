# StompRenderer

## Dependencies:
- glm
- glfw 3.3.4
- Vulkan
- stb_image
- Imgui
- tinyobjloader
- spdlog
- googletest

## Compile shaders
[Install glslc](https://vulkan-tutorial.com/Development_environment#page_Shader-Compiler) on linux

On linux
```bash
./compile_shaders.sh
```

## Build
```bash
mkdir cmake_build
cmake -S . -B cmake_build
cmake --build cmake_build
```

## Run
```bash
cd cmake_build
./renderer
```
