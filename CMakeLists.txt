cmake_minimum_required(VERSION 3.31)
project(c_playground)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# For clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -Wstrict-prototypes
)
# Debug
add_compile_options(-O0 -g)

option(BUILD_BEAN "Build bean" ON)
option(BUILD_SNAKE "Build snake" ON)

if(BUILD_BEAN)
    add_subdirectory(bean)
endif()

if(BUILD_SNAKE)
    add_subdirectory(snake)
endif()


