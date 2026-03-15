find_package(Qt6 COMPONENTS Widgets Concurrent REQUIRED)
find_package(Qt6 COMPONENTS Svg QUIET)
# 麦克风设备列表与采集由 lingjian_voice 提供（ALSA 或 Qt Multimedia），UI 不再直接依赖 Qt Multimedia

set(CMAKE_AUTOMOC ON)

add_executable(lingjian_ui
    main.cpp
    about_dialog.cpp
    about_dialog.h
    themed_message_box.cpp
    themed_message_box.h
    candidate_view.cpp
    candidate_view.h
    help_submenu.cpp
    help_submenu.h
    more_input_submenu.cpp
    more_input_submenu.h
    status_bar.cpp
    status_bar.h
    status_bar_menu.cpp
    status_bar_menu.h
    theme_manager.cpp
    theme_manager.h
    skin_loader.cpp
    skin_loader.h
    skin_selector.cpp
    skin_selector.h
    soft_keyboard.cpp
    soft_keyboard.h
    input_widget.cpp
    input_widget.h
    voice_input_dialog.cpp
    voice_input_dialog.h
)

target_link_libraries(lingjian_ui
    PRIVATE
        Qt6::Widgets
        Qt6::Concurrent
        lingjian_core
        lingjian_utils
        lingjian_voice
)
if(TARGET Qt6::Svg)
    target_link_libraries(lingjian_ui PRIVATE Qt6::Svg)
endif()

target_include_directories(lingjian_ui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
if(TARGET copy_vosk_models)
    add_dependencies(lingjian_ui copy_vosk_models)
endif()
