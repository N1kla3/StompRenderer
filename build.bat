call vcvarsall.bat x86_amd64 && cmake . --preset windows-msvc-debug-user-mode && cmake --build .\windows-msvc-debug-user-mode && cd .\windows-msvc-debug-user-mode && call renderer.exe
