cmake_minimum_required(VERSION 3.31)
project(snake C)

set(SNAKE_SOURCES
    main.c
    foo.c
)

add_executable(snake ${SNAKE_SOURCES})
