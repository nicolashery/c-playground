set(DODGE_SOURCES
    main.c
    foo.c
)

add_executable(dodge ${DODGE_SOURCES})

# Internal libs
target_link_libraries(dodge PRIVATE math_lib)

# External libs
target_link_libraries(dodge PRIVATE stb)
target_link_libraries(dodge PRIVATE raylib)

# Raylib: link appropriate frameworks on macOS
if (APPLE)
    target_link_libraries(dodge PRIVATE "-framework IOKit")
    target_link_libraries(dodge PRIVATE "-framework Cocoa")
    target_link_libraries(dodge PRIVATE "-framework OpenGL")
endif()
