#!/usr/bin/env bash
rm -r ./unixlike-gcc-debug && ./compile_shaders.sh && cmake . --preset unixlike-gcc-debug && cmake --build ./unixlike-gcc-debug && cd ./unixlike-gcc-debug && ctest -vv
