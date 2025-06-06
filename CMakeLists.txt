cmake_minimum_required(VERSION 3.31)
project(playground C)

set(CMAKE_C_STANDARD 23)

option(BUILD_BEAN "Build bean" ON)
option(BUILD_SNAKE "Build snake" ON)

if(BUILD_SNAKE)
    add_subdirectory(snake)
endif()

if(BUILD_BEAN)
    add_subdirectory(bean)
endif()
