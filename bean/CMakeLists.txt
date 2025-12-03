set(BEAN_SOURCES
    main.c
    foo.c
)

add_executable(bean ${BEAN_SOURCES})

add_custom_command(
        TARGET bean POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data)
