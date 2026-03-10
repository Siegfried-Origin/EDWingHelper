set(EDWINGHELPER_SOURCES
    main.cpp
    App.cpp
    GUI.cpp
    ../assets/edwinghelper.rc
)

if (WIN32)
    # GUI version
    add_executable(EDWingHelper WIN32 ${EDWINGHELPER_SOURCES})
else()
    # GUI version
    add_executable(EDWingHelper ${EDWINGHELPER_SOURCES})
endif()

target_include_directories(EDWingHelper PRIVATE ../3rdparty)
target_include_directories(EDWingHelper PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(EDWingHelper PRIVATE UNICODE _UNICODE)
target_link_libraries(EDWingHelper PRIVATE tinyui)

install(TARGETS EDWingHelper DESTINATION .)

