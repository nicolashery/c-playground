set(MATH_LIB_SOURCES
    math_utils.c
)

add_library(math_lib STATIC ${MATH_LIB_SOURCES})

target_include_directories(math_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
