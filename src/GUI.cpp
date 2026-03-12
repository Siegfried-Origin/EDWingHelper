#include "GUI.h"

#include "fonts/eurocaps.h"
#include "fonts/eurostile.h"
#include "fonts/icons.h"
#include "fonts/IconsMaterialDesign.h"

#include <algorithm>
#include <cstring>


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

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    float fontSize = style.FontScaleDpi * 22.f;
    float iconFontSize = 0.8 * fontSize;//style.FontScaleDpi * 24.f;

    // Add icon glyphs
    ImFontConfig cfgIcons;
    cfgIcons.MergeMode = true;
    cfgIcons.PixelSnapH = true;
    cfgIcons.GlyphMinAdvanceX = fontSize;
    cfgIcons.GlyphOffset.y = iconFontSize / 6.f;

    io.Fonts->AddFontFromMemoryCompressedTTF(
        Icons_compressed_data,
        Icons_compressed_size,
        iconFontSize,
        &cfgIcons
    );

    // Eurocaps
    float eurocapsFontSize = style.FontScaleDpi * 26.f;

    _fontEurocaps = io.Fonts->AddFontFromMemoryCompressedTTF(
        EUROCAPS_compressed_data,
        EUROCAPS_compressed_size,
        eurocapsFontSize
    );

    io.Fonts->AddFontFromMemoryCompressedTTF(
        Icons_compressed_data,
        Icons_compressed_size,
        iconFontSize,
        &cfgIcons
    );

    // Eurostile
    _fontEurostile = io.Fonts->AddFontFromMemoryCompressedTTF(
        Eurostile_compressed_data,
        Eurostile_compressed_size,
        fontSize
    );

    io.Fonts->AddFontFromMemoryCompressedTTF(
        Icons_compressed_data,
        Icons_compressed_size,
        iconFontSize,
        &cfgIcons
    );
}


GUI::~GUI()
{
    delete _mainWindow;
}


void GUI::run()
{
    while (!_mainWindow->closed()) {
        // Force confirmation in case commander were winged:
        // we cannot recover this list after closing
        // Do not update this field when a close is requested, otherwise
        // it interferes with the event loop.
        if (!_mainWindow->closeRequested()) {
            _mainWindow->allowClose(_app.getCmdrInvited().size() == 0);
        }

        if (!_mainWindow->minimized()) {
            _mainWindow->beginFrame();

            beginMainWindow();
            ImGui::PushFont(_fontEurostile);

            menuBar();

            if (_app.getCmdrList().size() == 0) {
                const char* importListText = ICON_MD_FOLDER_OPEN " Import List...";

                ImVec2 size = ImGui::CalcTextSize(importListText);
                float buttonWidth = size.x + ImGui::GetStyle().FramePadding.x * 2.0f;
                float buttonHeight = size.y + ImGui::GetStyle().FramePadding.y * 2.0f;

                ImVec2 avail = ImGui::GetContentRegionAvail();

                ImGui::SetCursorPosX((avail.x - buttonWidth) * 0.5f);
                ImGui::SetCursorPosY((avail.y - buttonHeight) * 0.5f);

                if (ImGui::Button(importListText)) {
                    _mainWindow->openCommanderListFileDialog(this, GUI::loadCommanderList);
                }
            }
            else {
                showCommanderLists();
            }

            showConfirmationMessages();

            ImGui::PopFont();

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
            if (ImGui::Button(ICON_MD_MINIMIZE, buttonSize)) { _mainWindow->minimizeWindow(); }

            ImGui::SetCursorPos(ImVec2(pViewport->Size.x - 2 * buttonWidth, 0.));
            if (ImGui::Button(ICON_MD_CHECK_BOX_OUTLINE_BLANK, buttonSize)) { _mainWindow->maximizeRestoreWindow(); }
        }
        ImGui::PopStyleColor(3);

        // Close button
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 125));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 50, 50, 200));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 0, 0, 255));
        {
            ImGui::SetCursorPos(ImVec2(pViewport->Size.x - buttonWidth, 0.));
            if (ImGui::Button(ICON_MD_CLOSE, buttonSize)) { _mainWindow->closeWindow(); }
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
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoResize);
}


void GUI::endMainWindow()
{
    ImGui::End();
}


void GUI::menuBar()
{
    bool allowSave = _app.getCmdrList().size() > 0;

    // TODO: currently shortcuts are not relable so deactivated
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal)) {
        _mainWindow->openCommanderListFileDialog(this, GUI::loadCommanderList);
    }

    if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal)) {
        _mainWindow->openCommanderListFileDialog(this, GUI::appendCommanderList);
    }

    if (allowSave && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal)) {
        _mainWindow->saveCommanderListFileDialog(this, GUI::exportCommanderList);
    }

    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_N, ImGuiInputFlags_RouteGlobal)) {
        _showAddCommander = true;
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open new commander list...")) {
                _mainWindow->openCommanderListFileDialog(this, GUI::loadCommanderList);
            }

            if (ImGui::MenuItem("Append commander list...")) {
                _mainWindow->openCommanderListFileDialog(this, GUI::appendCommanderList);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Export commander list...", NULL, false, allowSave)) {
                _mainWindow->saveCommanderListFileDialog(this, GUI::exportCommanderList);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Add commander...", "Ctrl+N")) {
                _showAddCommander = true;
            }

            ImGui::MenuItem("Show edit buttons", NULL, &_showEditButtons);

            //ImGui::Separator();

            //if (ImGui::MenuItem("Clear commander list", NULL, false, _app.getCmdrList().size() > 0)) {
            //    _confirmClearCmdrList = true;
            //}

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}


void GUI::showCommanderLists()
{
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

    const ImU32 rowOnline = ImGui::GetColorU32(ImVec4(0.0f, 0.2f, 0.0f, 0.65f));
    const ImU32 rowOffline = ImGui::GetColorU32(ImVec4(0.2f, 0.0f, 0.0f, 0.65f));
    const ImU32 rowInvited = ImGui::GetColorU32(ImVec4(0.0f, 0.1f, 0.2f, 0.65f));

    // ------------------------------------------------------------------------
    // Need invite
    // ------------------------------------------------------------------------

    const size_t totalNeedInvite = _app.getCmdrNeedInviteOnline().size() + _app.getCmdrNeedInviteOffline().size();

    ImGui::BeginChild("Need Invite", ImVec2(512, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
    ImGui::Text("Need invite (%d)", totalNeedInvite);
    ImGui::Separator();

    // Online
    if (ImGui::BeginTable("Online", 2, flags)) {
        std::string onlineText = "Online (" + std::to_string(_app.getCmdrNeedInviteOnline().size()) + ")";
        ImGui::TableSetupColumn(onlineText.c_str(), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        uint32_t uid = 0;

        const std::vector<std::string>& cmdrNeedsInviteOnline = _app.getCmdrNeedInviteOnline();

        ImGui::PushFont(_fontEurocaps);

        for (const std::string& cmdr : cmdrNeedsInviteOnline) {
            ImGui::TableNextRow();
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, rowOnline);

            ImGui::TableNextColumn();
            ImGui::Text(cmdr.c_str());

            ImGui::TableNextColumn();

            if (_showEditButtons) {
                ImGui::PushID(uid++);

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 125));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 50, 50, 200));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 0, 0, 255));

                if (ImGui::Button(ICON_MD_DELETE)) {
                    _app.removeCommander(cmdr);
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::PopFont();
                    ImGui::SetTooltip("Remove from list");
                    ImGui::PushFont(_fontEurocaps);
                }

                ImGui::PopStyleColor(3);

                ImGui::SameLine();

                if (ImGui::Button(ICON_MD_ARROW_FORWARD)) {
                    _app.setCmdrStatusInvited(cmdr);
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::PopFont();
                    ImGui::SetTooltip("Manualy move to the invited list");
                    ImGui::PushFont(_fontEurocaps);
                }

                ImGui::PopID();
            }
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }

    // Offline
    if (ImGui::BeginTable("Offline", 2, flags)) {
        std::string offlineText = "Offline (" + std::to_string(_app.getCmdrNeedInviteOffline().size()) + ")";
        ImGui::TableSetupColumn(offlineText.c_str(), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        uint32_t uid = 0;

        const std::vector<std::string>& cmdrNeedsInviteOffline = _app.getCmdrNeedInviteOffline();

        ImGui::PushFont(_fontEurocaps);

        for (const std::string& cmdr : cmdrNeedsInviteOffline) {
            ImGui::TableNextRow();
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, rowOffline);

            ImGui::TableNextColumn();
            ImGui::Text(cmdr.c_str());

            ImGui::TableNextColumn();

            if (_showEditButtons) {
                ImGui::PushID(uid++);

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 125));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 50, 50, 200));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 0, 0, 255));

                if (ImGui::Button(ICON_MD_DELETE)) {
                    _app.removeCommander(cmdr);
                }

                ImGui::PopStyleColor(3);

                if (ImGui::IsItemHovered()) {
                    ImGui::PopFont();
                    ImGui::SetTooltip("Remove from list");
                    ImGui::PushFont(_fontEurocaps);
                }

                ImGui::SameLine();

                if (ImGui::Button(ICON_MD_ARROW_FORWARD)) {
                    _app.setCmdrStatusInvited(cmdr);
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::PopFont();
                    ImGui::SetTooltip("Manualy move to the invited list");
                    ImGui::PushFont(_fontEurocaps);
                }

                ImGui::PopID();
            }
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }

    ImGui::EndChild();

    // ------------------------------------------------------------------------
    // Already invited
    // ------------------------------------------------------------------------

    const size_t totalInvited = _app.getCmdrInvited().size();

    ImGui::SameLine();
    ImGui::BeginChild("Invited", ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGui::Text("Already invited (%d)", totalInvited);
    ImGui::Separator();

    if (ImGui::BeginTable("In Wing", 2, flags)) {
        ImGui::TableSetupColumn("");
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        uint32_t uid = 0;
        const std::vector<std::string>& cmdrInvited = _app.getCmdrInvited();

        ImGui::PushFont(_fontEurocaps);

        for (const std::string& cmdr : cmdrInvited) {
            ImGui::TableNextRow();
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, rowInvited);

            ImGui::TableNextColumn();

            if (_showEditButtons) {
                ImGui::PushID(uid++);

                if (ImGui::Button(ICON_MD_ARROW_BACK)) {
                    _app.setCmdrStatusWaiting(cmdr);
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::PopFont();
                    ImGui::SetTooltip("Manualy remove from the invited list");
                    ImGui::PushFont(_fontEurocaps);
                }

                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 125));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 50, 50, 200));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 0, 0, 255));

                if (ImGui::Button(ICON_MD_DELETE)) {
                    _app.removeCommander(cmdr);
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::PopFont();
                    ImGui::SetTooltip("Remove from list");
                    ImGui::PushFont(_fontEurocaps);
                }

                ImGui::PopStyleColor(3);

                ImGui::PopID();
            }

            ImGui::TableNextColumn();
            ImGui::Text(cmdr.c_str());
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }

    ImGui::EndChild();
}


void GUI::showConfirmationMessages()
{
    static char inputName[255] = { 0 };

    // Close confirmation if needed
    if (_mainWindow->closeRequested() && _mainWindow->isCloseConfirmationRequired()) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("Confirm Exit");

        if (ImGui::BeginPopupModal("Confirm Exit", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(
                "You currently have invited commanders.\n"
                "Closing the application will cause you to lose the current winging progress.\n"
                "This action cannot be undone.\n"
                "\n"
                "Are you sure you want to exit the program ?\n"
            );

            float buttonWidth = 134.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float totalWidth = buttonWidth * 2 + spacing;

            float avail = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((avail - totalWidth) * 0.5f);

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                _mainWindow->resetCloseRequested();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("Confirm Exit", ImVec2(buttonWidth, 0))) {
                _mainWindow->allowClose(true);
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopStyleColor(3);

            ImGui::EndPopup();
        }
    }
    else if (_showAddCommander) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("Add a commander");

        if (ImGui::BeginPopupModal("Add a commander", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::IsWindowAppearing()) {
                ImGui::SetKeyboardFocusHere();
            }

            ImGui::Text("Commander name:");
            ImGui::SameLine();

            ImGui::PushFont(_fontEurocaps);

            // TODO: use a callback to show completion from known commanders in
            //       online friend list.
            if (ImGui::InputText("##CMDR_NAME", inputName, sizeof(inputName), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                _app.addCommander(inputName);
                _showAddCommander = false;
                std::memset(inputName, 0, sizeof(inputName));
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopFont();

            float buttonWidth = 134.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float totalWidth = buttonWidth * 2 + spacing;

            float avail = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((avail - totalWidth) * 0.5f);

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                _showAddCommander = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
                _app.addCommander(inputName);
                _showAddCommander = false;
                std::memset(inputName, 0, sizeof(inputName));
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopStyleColor(3);

            ImGui::EndPopup();
        }
    }
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


void GUI::appendCommanderList(void* userdata, std::string path)
{
    GUI* obj = (GUI*)userdata;

    // TODO: Handle errors and display a message
    if (!path.empty()) {
        //try {
        obj->_app.appendCommanderList(path);
        //}
        //catch (const std::runtime_error& e) {
        //    obj->_logErrStr = e.what();
        //    obj->_hasError = true;
        //}
    }
}


void GUI::exportCommanderList(void* userdata, std::string path)
{
    GUI* obj = (GUI*)userdata;

    if (!path.empty()) {
        std::filesystem::path fsPath(path);

        // Normalize extension to lowercase and check for ".txt"
        std::string ext = fsPath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

        if (ext != ".txt") {
            // Append ".txt" if the path does not already end with ".txt"
            path += ".txt";
            fsPath = std::filesystem::path(path);
        }

        obj->_app.exportCommanderList(fsPath);
    }
}