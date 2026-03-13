add_library(lingjian_core STATIC
    context.cpp
    decoder.cpp
    dictionary.cpp
    pinyin_segmenter.cpp
    language_model.cpp
    sentence_decoder.cpp
)

target_include_directories(lingjian_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

