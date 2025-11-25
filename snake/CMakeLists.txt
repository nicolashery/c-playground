set(SNAKE_SOURCES
    main.c
    foo.c
)

add_executable(snake ${SNAKE_SOURCES})

# Internal libs
target_link_libraries(snake PRIVATE math_lib)

# External libs
target_link_libraries(snake PRIVATE stb)
