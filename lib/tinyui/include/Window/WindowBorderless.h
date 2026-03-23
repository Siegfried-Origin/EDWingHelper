#pragma once

#include <Window/Window.h>


class WindowBorderless : public Window
{
public:
    WindowBorderless(
        WindowSystem* sys,
        const std::string& title,
        const std::filesystem::path& config,
        uint32_t width = 640,
        uint32_t height = 700
    );

    virtual ~WindowBorderless();

    void minimizeWindow();
    void maximizeRestoreWindow();

    float titleBarHeight() const { return _titlebarHeight; }
    float windowButtonWidth() const { return _buttonWidth; }
    bool borderless() const { return _borderless; }

protected:
    float _titlebarHeight = 32.f;
    float _buttonWidth = 55.f;
    float _totalButtonWidth = 3 * 55.f;
    bool _borderless = true;

#ifdef USE_SDL
    static SDL_HitTestResult SDLCALL sdlHitTest(SDL_Window* win, const SDL_Point* area, void* data);

    bool _isMaximized = false;
#else
    // WIN32 stuff for working with borderless windows
    // see https://github.com/melak47/BorderlessWindow/tree/main
    virtual LRESULT w32WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    virtual DWORD w32Style();

    static bool w32CompositionEnabled();

    void w32SetBorderless(bool borderless);
    bool w32IsMaximized();
    void w32AdjustMaximizedClientRect(RECT& rect);
    LRESULT w32HitTest(POINT cursor) const;
#endif
};