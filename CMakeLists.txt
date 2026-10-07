cmake_minimum_required(VERSION 3.16)
project(microbeworld LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(microbeworld STATIC microbeworld.cpp)
target_compile_options(microbeworld PRIVATE -O3)

# Emscripten browser build:
# emcmake cmake -S . -B build
# emmake cmake --build build
# Or compile directly as documented in README.md.