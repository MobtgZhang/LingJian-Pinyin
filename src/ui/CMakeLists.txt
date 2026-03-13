find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(lingjian_ui
    main.cpp
    candidate_view.cpp
    candidate_view.h
    status_bar.cpp
    status_bar.h
    status_bar_menu.cpp
    status_bar_menu.h
    theme_manager.cpp
    theme_manager.h
)

target_link_libraries(lingjian_ui
    PRIVATE
        Qt6::Widgets
)

target_include_directories(lingjian_ui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

