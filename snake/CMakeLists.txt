set(SNAKE_SOURCES
    main.c
    foo.c
)

add_executable(snake ${SNAKE_SOURCES})

# Internal libs
target_link_libraries(snake PRIVATE math_lib)

# External libs
target_link_libraries(snake PRIVATE stb)
target_link_libraries(snake PRIVATE raylib)

# Raylib: link appropriate frameworks on macOS
if (APPLE)
    target_link_libraries(snake PRIVATE "-framework IOKit")
    target_link_libraries(snake PRIVATE "-framework Cocoa")
    target_link_libraries(snake PRIVATE "-framework OpenGL")
endif()
