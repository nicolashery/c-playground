set(DODGE_SOURCES
    main.c
)

add_executable(dodge ${DODGE_SOURCES})

target_link_libraries(dodge PRIVATE raylib)

# Raylib: link appropriate frameworks on macOS
if (APPLE)
    target_link_libraries(dodge PRIVATE "-framework IOKit")
    target_link_libraries(dodge PRIVATE "-framework Cocoa")
    target_link_libraries(dodge PRIVATE "-framework OpenGL")
endif()

add_custom_command(
        TARGET dodge POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/assets)
