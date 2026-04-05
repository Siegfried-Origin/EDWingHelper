#pragma once

#include "App.h"

#include <Window/WindowSystem.h>
#include <Window/WindowBorderless.h>

#ifndef BUILD_OVERLAY
#include <Window/WindowOverlay.h>
#endif

#include <filesystem>

class GUI
{
public:
    GUI(
        const std::filesystem::path& config,
        WindowSystem* pWindowSystem
    );

    ~GUI();

    void run();

private:
    void showMainWindowContent();
#ifdef BUILD_OVERLAY
    void showOverlayContent();
#endif

    void beginMainWindow();
    void endMainWindow();

    void menuBar();
    
    void showCommanderLists(ImFont* font);

    void showToInviteList(
        const char* title_name,
        const std::vector<std::string>& list,
        const ImU32& bgColor,
        ImFont* font
    );

    void showInstancedList(
        const char* title_name,
        const std::vector<std::string>& list,
        const ImU32& bgColor,
        ImFont* font
    );

    void showConfirmationMessages();


    void openNewCommanderListDialog();
    void appendCommanderListDialog();
    void saveCommanderListDialog();

    static void loadCommanderList(void* userdata, std::string path);
    static void appendCommanderList(void* userdata, std::string path);
    static void exportCommanderList(void* userdata, std::string path);

private:
    App _app;
    WindowSystem* _pWindowSystem = nullptr;

    WindowBorderless* _mainWindow = nullptr;
    ImFont* _fontMainWindowEurocaps = nullptr;
    ImFont* _fontMainWindowEurostile = nullptr;

#ifdef BUILD_OVERLAY
    WindowOverlay* _overlayWindow = nullptr;
    ImFont* _fontOverlayEurocaps = nullptr;
    ImFont* _fontOverlayEurostile = nullptr;
#endif

    bool _showAddCommander = false;
    bool _showEditButtons = false;
    bool _confirmClearCmdrList = false;
};
