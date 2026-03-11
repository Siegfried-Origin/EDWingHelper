#pragma once

#include "App.h"

#include <Window/WindowSystem.h>
#include <Window/WindowBorderless.h>

#include <filesystem>

class GUI
{
public:
    GUI(
        const std::filesystem::path& execPath,
        WindowSystem* pWindowSystem
    );

    ~GUI();

    void run();

private:
    void beginMainWindow();
    void endMainWindow();

    void showCommanderLists();

    static void loadCommanderList(void* userdata, std::string path);

private:
    App _app;
    WindowSystem* _pWindowSystem = nullptr;
    WindowBorderless* _mainWindow = nullptr;
};