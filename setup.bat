curl -LO https://github.com/glfw/glfw/releases/download/3.3.9/glfw-3.3.9.zip
unzip .\glfw-3.3.9.zip
del glfw-3.3.9.zip
git clone https://github.com/nothings/stb.git
copy .\stb\stb_image.h .\src
rmdir stb
git submodule update --init

cmake . --preset windows-msvc-debug-user-mode
cmake --build