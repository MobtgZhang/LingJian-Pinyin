cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core Concurrent REQUIRED)
find_package(Qt6 COMPONENTS Multimedia QUIET)

# Linux 下优先使用 ALSA 采集麦克风（更轻量，不依赖 Qt Multimedia）
find_package(ALSA QUIET)
if(UNIX AND NOT APPLE AND ALSA_FOUND)
    set(LINGJIAN_HAVE_ALSA 1)
endif()

set(CMAKE_AUTOMOC ON)

set(VOICE_SOURCES
    vosk_speech_engine.cpp
    vosk_speech_engine.h
)
if(LINGJIAN_HAVE_ALSA)
    list(APPEND VOICE_SOURCES alsa_capture_thread.cpp alsa_capture_thread.h)
endif()
add_library(lingjian_voice STATIC ${VOICE_SOURCES})

target_include_directories(lingjian_voice
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lingjian_voice
    PUBLIC
        Qt6::Core
        Qt6::Concurrent
)

if(LINGJIAN_HAVE_ALSA)
    target_link_libraries(lingjian_voice PUBLIC ALSA::ALSA)
    target_compile_definitions(lingjian_voice PUBLIC LINGJIAN_HAVE_ALSA=1)
endif()

if(TARGET Qt6::Multimedia)
    target_link_libraries(lingjian_voice PUBLIC Qt6::Multimedia)
    target_compile_definitions(lingjian_voice PUBLIC LINGJIAN_HAVE_QT_MULTIMEDIA=1)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(lingjian_voice
        PRIVATE
            dl
    )
endif()

# 语音输入测试程序：打开麦克风并说话，识别到内容即表示测试成功
option(BUILD_VOICE_TEST "Build voice input test executable" ON)
if(BUILD_VOICE_TEST)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)
    add_executable(voice_input_test voice_input_test.cpp)
    target_link_libraries(voice_input_test
        PRIVATE
            lingjian_voice
            Qt6::Core
            Qt6::Widgets
    )
    target_include_directories(voice_input_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    if(TARGET copy_vosk_models)
        add_dependencies(voice_input_test copy_vosk_models)
    endif()
endif()
