#!/usr/bin/env bash
cmake . --preset unixlike-gcc-debug && cmake --build ./unixlike-gcc-debug && cd ./unixlike-gcc-debug && ./renderer.exe
