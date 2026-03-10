set(TINYUI_SOURCES
    src/Window/WindowSystem.cpp
    src/Window/Window.cpp
    src/Window/WindowBorderless.cpp
    src/Window/WindowOverlay.cpp
)

if (USE_VULKAN)
    set(TINYUI_SOURCES ${TINYUI_SOURCES}
        src/Vulkan/VkAdapter.cpp
        src/Vulkan/Swapchain.cpp
        src/Vulkan/VkUtil.cpp
    )
else ()
    set(TINYUI_SOURCES ${TINYUI_SOURCES}
        src/DX11/DX11Adapter.cpp
    )
endif()

add_library(tinyui ${TINYUI_SOURCES})
target_link_libraries(tinyui PUBLIC imgui)
target_include_directories(tinyui PUBLIC include)
target_include_directories(tinyui PRIVATE ../3rdparty) # TODO: move config.h.in
target_include_directories(tinyui PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(tinyui PRIVATE UNICODE _UNICODE)

if (USE_SDL)
    target_link_libraries(tinyui PRIVATE SDL3::SDL3)
endif()

install(TARGETS tinyui DESTINATION .)

if (WIN32 AND USE_SDL)
    install(FILES $<TARGET_FILE:SDL3::SDL3-shared> DESTINATION .)
endif()
