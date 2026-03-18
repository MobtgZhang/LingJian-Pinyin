add_library(lingjian_utils STATIC
    sc_tc_converter.cpp
)

target_include_directories(lingjian_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

