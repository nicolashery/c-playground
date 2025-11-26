set(MATH_LIB_SOURCES
    math_utils.c
)

add_library(math_lib STATIC ${MATH_LIB_SOURCES})

target_include_directories(math_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(BUILD_TESTING)
    set(MATH_LIB_TESTS
        test_math_utils.c
    )

    add_executable(test_math_lib ${MATH_LIB_TESTS})
    target_link_libraries(test_math_lib PRIVATE math_lib unity)

    add_test(NAME math_lib_tests COMMAND test_math_lib)
endif()
