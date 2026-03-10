#include "GUI.h"


GUI::GUI(
    const std::filesystem::path& execPath,
    WindowSystem* pWindowSystem)
    : _app(execPath)
    , _pWindowSystem(pWindowSystem)
{
    _mainWindow = new WindowBorderless(
        _pWindowSystem,
        "ED Wing Helper",
        execPath / "imgui.ini"
    );
}


GUI::~GUI()
{
    delete _mainWindow;
}


void GUI::run()
{
    while (!_mainWindow->closed()) {
        if (!_mainWindow->minimized()) {
            _mainWindow->beginFrame();

            beginMainWindow();

            endMainWindow();

            _mainWindow->endFrame();
        }
    }
}


void GUI::beginMainWindow()
{
    ImGuiViewport* pViewport = ImGui::GetMainViewport();

    if (_mainWindow->borderless()) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

        ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, _mainWindow->titleBarHeight()));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Title", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoDecoration);

        ImGui::SetScrollY(0.0f);
        ImGui::SetScrollX(0.0f);

        const ImGuiStyle& style = ImGui::GetStyle();
        const float titleMarginLeft = 8.f;
        const float buttonWidth = _mainWindow->windowButtonWidth();
        const ImVec2 buttonSize(buttonWidth, _mainWindow->titleBarHeight());

        // Center title vertically
        ImGui::PushFont(NULL, style.FontSizeBase * 1.2f);
        ImGui::SetCursorPos(ImVec2(titleMarginLeft, .5f * (_mainWindow->titleBarHeight() - ImGui::GetFontSize())));
        ImGui::Text("%s", _mainWindow->title());
        ImGui::PopFont();

        // Minimize & Resize buttons
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 20));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 50));
        {
            ImGui::SetCursorPos(ImVec2(pViewport->Size.x - 3 * buttonWidth, 0.));
            if (ImGui::Button("-", buttonSize)) { _mainWindow->minimizeWindow(); }

            ImGui::SetCursorPos(ImVec2(pViewport->Size.x - 2 * buttonWidth, 0.));
            if (ImGui::Button("+", buttonSize)) { _mainWindow->maximizeRestoreWindow(); }
        }
        ImGui::PopStyleColor(3);

        // Close button
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 125));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 50, 50, 200));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 0, 0, 255));
        {
            ImGui::SetCursorPos(ImVec2(pViewport->Size.x - buttonWidth, 0.));
            if (ImGui::Button("x", buttonSize)) { _mainWindow->closeWindow(); }
        }
        ImGui::PopStyleColor(3);

        // Title separator
        ImU32 col = ImGui::GetColorU32(ImGuiCol_Separator);

        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(0, _mainWindow->titleBarHeight()),
            ImVec2(pViewport->Size.x, _mainWindow->titleBarHeight()),
            col, 1.0f
        );

        ImGui::End();
        ImGui::PopStyleVar(3);

        ImGui::SetNextWindowSize(ImVec2(pViewport->Size.x, pViewport->Size.y - _mainWindow->titleBarHeight()));
        ImGui::SetNextWindowPos(ImVec2(0, _mainWindow->titleBarHeight()));
    }
    else {
        ImGui::SetNextWindowSize(pViewport->Size);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
    }

    ImGui::Begin("Main", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize);
}


void GUI::endMainWindow()
{
    ImGui::End();
}