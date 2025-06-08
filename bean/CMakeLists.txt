cmake_minimum_required(VERSION 3.31)
project(bean C)

add_compile_options(-Wall -Wextra -pedantic -Werror)

add_executable(bean main.c)

add_custom_command(
        TARGET bean POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data)
