#pragma once

#include "App.h"

#include <Window/WindowSystem.h>
#include <Window/WindowBorderless.h>
#include <Window/WindowOverlay.h>

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
    void beginMainWindow();
    void endMainWindow();

    void displayToInviteList(
        const char* title_name,
        const std::vector<std::string>& list,
        const ImU32& bgColor
    );

    void displayInstancedList(
        const char* title_name,
        const std::vector<std::string>& list,
        const ImU32& bgColor
    );

    void menuBar();

    void showCommanderLists();
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
    WindowOverlay* _overlayWindow = nullptr;

    ImFont* _fontEurocaps = nullptr;
    ImFont* _fontEurostile = nullptr;

    bool _showAddCommander = false;
    bool _showEditButtons = false;
    bool _confirmClearCmdrList = false;
};