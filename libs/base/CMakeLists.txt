set(BASE_SOURCES
    arena.c
    pool.c
    strings.c
)

add_library(base STATIC ${BASE_SOURCES})

target_include_directories(base PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(BUILD_TESTING)
    add_executable(test_arena test_arena.c)
    target_link_libraries(test_arena PRIVATE base unity)
    add_test(NAME arena_tests COMMAND test_arena)

    add_executable(test_pool test_pool.c)
    target_link_libraries(test_pool PRIVATE base unity)
    add_test(NAME pool_tests COMMAND test_pool)

    add_executable(test_strings test_strings.c)
    target_link_libraries(test_strings PRIVATE base unity)
    add_test(NAME strings_tests COMMAND test_strings)
endif()
