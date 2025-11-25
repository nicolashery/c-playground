cmake_minimum_required(VERSION 3.31)
project(c_playground)

include(FetchContent)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# For clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -Wstrict-prototypes
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
endif()

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG f1c79c02822848a9bed4315b12c8c8f3761e1296 # 2025-10-25
)

FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)
# Single-header file libs compiled in same translation unit as projects
# Mark as SYSTEM so they don't generate compiler warnings
target_include_directories(stb SYSTEM INTERFACE ${stb_SOURCE_DIR})

option(BUILD_BEAN "Build bean" ON)
option(BUILD_SNAKE "Build snake" ON)

add_subdirectory(libs/math_lib)

if(BUILD_BEAN)
    add_subdirectory(bean)
endif()

if(BUILD_SNAKE)
    add_subdirectory(snake)
endif()


