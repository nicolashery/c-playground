add_executable(scratch main.c)

add_custom_command(
        TARGET scratch POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data)
