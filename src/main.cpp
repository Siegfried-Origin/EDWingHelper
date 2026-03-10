#include "GUI.h"

#include <Window/WindowSystem.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#endif


#ifdef WIN32
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    LPWSTR* argv;
    int argc;
    argv = CommandLineToArgvW(GetCommandLine(), &argc);
#else
int main(int argc, char* argv[])
{
#endif
    const std::filesystem::path execPath = std::filesystem::path(argv[0]).parent_path();

    // Keep logs
    std::ostringstream local;
    std::cout.rdbuf(local.rdbuf());
    std::cerr.rdbuf(local.rdbuf());

    bool writeCrashLog = false;

    try {
#ifdef USE_SDL
        WindowSystem windowSystem;
#else
        WindowSystem windowSystem(hInstance, nShowCmd);
#endif
        GUI gui(execPath, &windowSystem);
        gui.run();
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL ] Exception: " << e.what() << std::endl;
        writeCrashLog = true;
    }

    // Write log in case of crash
    if (writeCrashLog) {
        std::ofstream out("EDVoiceCrash.log", std::ios::app);
        out << local.str();
        out.close();
    }

    return 0;
}