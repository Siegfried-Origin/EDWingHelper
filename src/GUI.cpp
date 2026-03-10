#include "GUI.h"

#include <array>

GUI::GUI(
    const std::filesystem::path& execPath,
    WindowSystem* pWindowSystem)
    : _app(execPath)
    , _pWindowSystem(pWindowSystem)
{
    _mainWindow = new WindowBorderless(
        _pWindowSystem,
        "ED Wing Helper",
        execPath / "imgui.ini",
        1024, 768
    );
}


GUI::~GUI()
{
    delete _mainWindow;
}


void GUI::run()
{
    std::array<std::vector<std::string>, App::N_STATUS> cmdrTracker;

    while (!_mainWindow->closed()) {
        if (!_mainWindow->minimized()) {
            _mainWindow->beginFrame();

            beginMainWindow();

            if (ImGui::Button("Import List...")) {
                _mainWindow->openCommanderListFileDialog(this, GUI::loadCommanderList);
            }

            // Update status list
            for (std::vector<std::string>& list : cmdrTracker) {
                list.clear();
            }

            const std::map<std::string, App::Status>& cmdrList = _app.getCmdrList();

            for (auto const& [cmdr, status] : cmdrList) {
                cmdrTracker[status].push_back(cmdr);
            }

            static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

            ImGui::BeginChild("Need Invite", ImVec2(512, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            ImGui::Text("Need invite");
            ImGui::Separator();

            if (ImGui::BeginTable("Online", 2, flags)) {
                ImGui::TableSetupColumn("Online", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("");
                ImGui::TableHeadersRow();

                for (const std::string& cmdr : cmdrTracker[App::NeedsInvite_Online]) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text(cmdr.c_str());

                    ImGui::TableNextColumn();
                }

                ImGui::EndTable();
            }

            if (ImGui::BeginTable("Offline", 2, flags)) {
                ImGui::TableSetupColumn("Offline", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("");
                ImGui::TableHeadersRow();

                for (const std::string& cmdr : cmdrTracker[App::NeedsInvite_Offline]) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text(cmdr.c_str());

                    ImGui::TableNextColumn();
                }

                ImGui::EndTable();
            }

            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("Invited", ImVec2(0, 0), ImGuiChildFlags_Borders);
            ImGui::Text("Already invited");
            ImGui::Separator();

            if (ImGui::BeginTable("In Wing", 2, flags)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("");
                ImGui::TableHeadersRow();

                for (const std::string& cmdr : cmdrTracker[App::Invited]) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text(cmdr.c_str());

                    ImGui::TableNextColumn();
                }

                ImGui::EndTable();
            }

            ImGui::EndChild();

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



void GUI::loadCommanderList(void* userdata, std::string path)
{
    GUI* obj = (GUI*)userdata;

    // TODO: Handle errors and display a message
    if (!path.empty()) {
        //try {
        obj->_app.loadCommanderList(path);
        //}
        //catch (const std::runtime_error& e) {
        //    obj->_logErrStr = e.what();
        //    obj->_hasError = true;
        //}
    }
}