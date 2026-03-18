add_subdirectory(core)
add_subdirectory(utils)

if(BUILD_UI)
    add_subdirectory(voice)
endif()

if(BUILD_FCITX5)
    add_subdirectory(fcitx)
endif()

if(BUILD_UI)
    add_subdirectory(ui)
endif()
