set(SNAKE_SOURCES
    main.c
    foo.c
)

add_executable(snake ${SNAKE_SOURCES})

target_link_libraries(snake PRIVATE math_lib)
