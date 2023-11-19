cmake_minimum_required(VERSION 3.27.1)

project(example CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(example)

target_sources(
    example
    PRIVATE
    main.cpp
)

target_include_directories(
    example
    PUBLIC
    ../
)
