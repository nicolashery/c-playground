set(BASE_SOURCES
    arena.c
)

add_library(base STATIC ${BASE_SOURCES})

target_include_directories(base PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(BUILD_TESTING)
    add_executable(test_arena test_arena.c)
    target_link_libraries(test_arena PRIVATE base unity)
    add_test(NAME arena_tests COMMAND test_arena)
endif()
