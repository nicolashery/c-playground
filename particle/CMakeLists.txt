set(PARTICLE_SOURCES
    main.c
)

add_executable(particle ${PARTICLE_SOURCES})

target_link_libraries(particle PRIVATE raylib)

# Raylib: link appropriate frameworks on macOS
if (APPLE)
    target_link_libraries(particle PRIVATE "-framework IOKit")
    target_link_libraries(particle PRIVATE "-framework Cocoa")
    target_link_libraries(particle PRIVATE "-framework OpenGL")
endif()
