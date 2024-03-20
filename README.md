# StompRenderer

## Dependencies:
- glm
- glfw 3.3.8
- Vulkan
- stb_image
- Imgui - docking branch
- tinyobjloader
- spdlog
- googletest
- nlohmann/json version 3.10.5

## Compile shaders
[Install glslc](https://vulkan-tutorial.com/Development_environment#page_Shader-Compiler) on linux

On linux
```bash
./compile_shaders.sh
```
On Windows
```bash
.\compile_shaders.bat
```

## Build Linux/Windows
Presets located in CMakePresets.json file.
Existing presets:
- "windows-msvc-debug-user-mode"
- "windows-mingw-debug-user-mode"
- "windows-clang-debug"
- "unixlike-gcc-debug"
- "unixlike-clang-debug"
  
```bash
cmake . --preset "name of the preset"
cmake --build "name of the preset"
```

## Run
### Linux
```bash
cd "name of the preset"
./renderer
```
### Windows
```bash
cd "name of the preset"
.\renderer
```
