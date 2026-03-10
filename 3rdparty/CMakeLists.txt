add_subdirectory(imgui)

if (USE_SDL)
    if (WIN32)
        set(SDL_SHARED ON CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
    endif()
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

    add_subdirectory(SDL)
endif()
