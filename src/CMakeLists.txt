add_subdirectory(fcitx)
add_subdirectory(core)
add_subdirectory(dict)
add_subdirectory(utils)

if(BUILD_UI)
    add_subdirectory(ui)
endif()

