cmake_minimum_required(VERSION 3.15)

project(EDWingMaker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    option(USE_SDL "Use SDL library for window management backend" OFF)
    option(USE_VULKAN "Use Vulkan API" OFF)
else()
    set(USE_SDL ON CACHE BOOL "Use SDL library for window management backend" FORCE)
    set(USE_VULKAN ON CACHE BOOL "Use Vulkan API" FORCE)
endif()

include(GNUInstallDirs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/plugins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Configuration
configure_file(cmake/config.h.in config.h @ONLY)

# Build
add_subdirectory(3rdparty)
add_subdirectory(lib)
add_subdirectory(src)

# CPack
set(CPACK_PACKAGE_NAME "ED Wing Maker")
set(CPACK_PACKAGE_VENDOR "Siegfried Origin")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP")
# Remove source, can be problematic on WIN32 with SDL_Mixer
set(CPACK_SOURCE_GENERATOR "")

execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(GIT_TAG)
    set(CPACK_PACKAGE_VERSION "${GIT_TAG}")
else()
    set(CPACK_PACKAGE_VERSION "0.0.0")
endif()

include(CPack)
