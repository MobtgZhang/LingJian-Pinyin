add_library(lingjian_fcitx STATIC
    addon.cpp
    engine.cpp
)

target_include_directories(lingjian_fcitx
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# 这里暂时不直接链接 Fcitx5，后续根据实际开发环境添加：
# find_package(Fcitx5Core REQUIRED)
# target_link_libraries(lingjian_fcitx PRIVATE Fcitx5::Core)

