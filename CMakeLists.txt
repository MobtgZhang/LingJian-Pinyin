cmake_minimum_required(VERSION 3.16)
project(LingJianPinyin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 所有目标启用 PIC，确保静态库能链接到共享库（fcitx5 addon）
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_UI "Build standalone UI process" ON)
option(BUILD_FCITX5 "Build Fcitx5 input method addon" ON)

if(BUILD_FCITX5)
    find_package(Fcitx5Core 5.0 QUIET)
    if(NOT Fcitx5Core_FOUND)
        message(WARNING
            "Fcitx5Core not found. Install libfcitx5core-dev (Ubuntu/Debian) "
            "or fcitx5-devel (Fedora). Skipping fcitx5 addon build.")
        set(BUILD_FCITX5 OFF)
    else()
        message(STATUS "Fcitx5 found — will build input method addon")
        message(STATUS "  FCITX_INSTALL_ADDONDIR   = ${FCITX_INSTALL_ADDONDIR}")
        message(STATUS "  FCITX_INSTALL_PKGDATADIR = ${FCITX_INSTALL_PKGDATADIR}")
    endif()
endif()

# 构建时把源码目录的 data/vosk-models 复制到 build/data/vosk-models，
# 以便从 build 目录运行 lingjian_ui 或 voice_input_test 时能直接找到模型
set(VOSK_MODELS_SRC "${CMAKE_SOURCE_DIR}/data/vosk-models")
set(VOSK_MODELS_BUILD "${CMAKE_BINARY_DIR}/data/vosk-models")
if(EXISTS "${VOSK_MODELS_SRC}")
    add_custom_target(copy_vosk_models ALL
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${VOSK_MODELS_SRC}" "${VOSK_MODELS_BUILD}"
        COMMENT "复制 Vosk 模型到 build/data/vosk-models"
    )
endif()

add_subdirectory(src)
