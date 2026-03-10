#pragma once

#include <filesystem>

class App
{
public:
    App(const std::filesystem::path& execPath);

    ~App();
};