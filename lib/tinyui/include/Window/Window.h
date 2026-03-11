#pragma once

#include "WindowSystem.h"

#include <string>
#include <filesystem>
#include <config.h>

#include <imgui.h>

#ifdef USE_SDL
    #include <SDL3/SDL.h>
#else
    #include <windows.h>
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef USE_VULKAN
    #include <Vulkan/VkAdapter.h>
#else
    #include <DX11/DX11Adapter.h>
#endif



class Window
{
protected:
    Window(
        WindowSystem* sys,
        const std::string& title,
        const std::filesystem::path& config,
        uint32_t width = 640,
        uint32_t height = 700
    );
public:
    virtual ~Window();

    virtual void beginFrame();
    virtual void endFrame();

    void closeWindow();
    bool closeRequested() { return _closeRequested; }
    void resetCloseRequested() { _closeRequested = false; }
    bool isCloseConfirmationRequired() { return !_allowClose; }

    // Allow closing the window without confirmation
    // if a close request is pending, close the window
    void allowClose(bool allowClose);

    bool closed() const { return _closed; }
    bool minimized() const { return _minimized; }
    float mainScale() const { return _mainScale; }
    const char* title() const;

#ifdef USE_VULKAN
    void createVkSurfaceKHR(
        VkInstance instance,
        VkSurfaceKHR* surface, int* width, int* height) const;
#endif

#ifdef USE_SDL
    SDL_Window* handle() const { return _sdlWindow; }
#else
    HWND handle() const { return _hwnd; }
#endif

protected:
    void onResize(uint32_t width, uint32_t height);
    void refreshResize();

    const std::filesystem::path _configPath;

#ifdef USE_VULKAN
    VkAdapter _gpuAdapter;
#else
    DX11Adapter _gpuAdapter;
#endif
    bool _gpuInitialized = false;

    WindowSystem* _sys = nullptr;
    ImGuiContext* _imGuiContext = nullptr;
    bool _imGuiInitialized = false;

    // GUI properties
    bool _closed = false;
    bool _minimized = false;
    float _mainScale = 1.f;
    std::string _title;

    // Allow for confirmation before closing the window
    bool _allowClose = true;
    bool _closeRequested = false;

#ifdef USE_SDL
    virtual void sdlWndProc(SDL_Event& event);
    SDL_Window* _sdlWindow;
#else
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT w32WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    HWND _hwnd;
    std::wstring _className;
#endif
};
