add_library(lingjian_dict STATIC
    dictionary_placeholder.cpp
)

target_include_directories(lingjian_dict
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

