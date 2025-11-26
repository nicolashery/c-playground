set(MATH_LIB_SOURCES
    math_utils.c
)

add_library(math_lib STATIC ${MATH_LIB_SOURCES})

target_include_directories(math_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(BUILD_TESTING)
    add_executable(test_math_utils test_math_utils.c)
    target_link_libraries(test_math_utils PRIVATE math_lib unity)
    add_test(NAME math_utils_tests COMMAND test_math_utils)

    add_executable(test_foo test_foo.c)
    target_link_libraries(test_foo PRIVATE math_lib unity)
    add_test(NAME foo_tests COMMAND test_foo)
endif()
