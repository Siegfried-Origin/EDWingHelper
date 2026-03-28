#include "GUI.h"

#include "fonts/eurocaps.h"
#include "fonts/eurostile.h"
#include "fonts/icons.h"
#include "fonts/IconsMaterialDesign.h"

#include <algorithm>
#include <cstring>


GUI::GUI(
    const std::filesystem::path& config,
    WindowSystem* pWindowSystem)
    : _app(config)
    , _pWindowSystem(pWindowSystem)
{
    _mainWindow = new WindowBorderless(
        _pWindowSystem,
        "ED Wing Helper",
        config / "mainwindow.ini",
        1024, 768
    );

    ImGui::SetCurrentContext(_mainWindow->getImGuiContext());

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

    //_overlayWindow = new WindowOverlay(
    //    _pWindowSystem,
    //    "ED Wing Helper overlay",
    //    config / "overlay.ini",
    //    L"EliteDangerous64.exe"
    //    //L"notepad.exe"
    //);

    //_overlayWindow->setShown(false);
}


GUI::~GUI()
{
    delete _mainWindow;

    if (_overlayWindow) {
        _overlayWindow->closeWindow();
        // TODO: must be solved, it seems the join blocks
        // the delete
        //delete _overlayWindow;
    }
}


void GUI::run()
{
    const float target_ms = 16.6f;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!_mainWindow->closed()) {
        const auto now = std::chrono::high_resolution_clock::now();
        const float delta_ms = std::chrono::duration<float, std::milli>(now - lastTime).count();

        // Wait if last frame was rendered early to lower GPU utilization
        if (delta_ms < target_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(target_ms - delta_ms)));
            continue;
        }

        lastTime = now;

        if (_overlayWindow && _overlayWindow->active()) {
            _overlayWindow->beginFrame();
            ImGui::ShowDemoWindow();
            _overlayWindow->endFrame();
        }

        // Force confirmation in case commander were winged:
        // we cannot recover this list after closing
        // Do not update this field when a close is requested, otherwise
        // it interferes with the event loop.
        if (!_mainWindow->closeRequested()) {
            const bool hasCommandersInWing = _app.getCmdrInvitedConfirmed().size() + _app.getCmdrInvitedUnconfirmed().size() > 0;
            const bool hasUnsavedEdits = _app.wasEditedSinceLastSave();
            _mainWindow->allowClose(!hasCommandersInWing && !hasUnsavedEdits);
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
                    openNewCommanderListDialog();
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
        openNewCommanderListDialog();
    }

    if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal)) {
        appendCommanderListDialog();
    }

    if (allowSave && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal)) {
        saveCommanderListDialog();
    }

    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_N, ImGuiInputFlags_RouteGlobal)) {
        _showAddCommander = true;
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open new commander list...")) {
                openNewCommanderListDialog();
            }

            if (ImGui::MenuItem("Append commander list...")) {
                appendCommanderListDialog();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Export commander list...", NULL, false, allowSave)) {
                saveCommanderListDialog();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                _mainWindow->closeWindow();
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
    const ImU32 rowInvitedConfirmed = ImGui::GetColorU32(ImVec4(0.0f, 0.1f, 0.35f, 0.65f));
    const ImU32 rowInvitedUnconfirmed = ImGui::GetColorU32(ImVec4(0.0f, 0.1f, 0.2, 0.65f));

    // ------------------------------------------------------------------------
    // Need invite
    // ------------------------------------------------------------------------

    const size_t totalNeedInvite = _app.getCmdrNeedInviteOnline().size() + _app.getCmdrNeedInviteOffline().size();

    // To be invited commandes
    ImGui::BeginChild("Need Invite", ImVec2(512, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
    ImGui::Text("Need invite (%d)", totalNeedInvite);
    ImGui::Separator();

    displayToInviteList("Online", _app.getCmdrNeedInviteOnline(), rowOnline);
    displayToInviteList("Offline", _app.getCmdrNeedInviteOffline(), rowOffline);

    ImGui::EndChild();

    // ------------------------------------------------------------------------
    // Instanced
    // ------------------------------------------------------------------------

    const size_t totalInvited = _app.getCmdrInvitedConfirmed().size() + _app.getCmdrInvitedUnconfirmed().size();

    ImGui::SameLine();
    ImGui::BeginChild("Invited", ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGui::Text("Instanced (%d)", totalInvited);
    ImGui::Separator();

    displayInstancedList("Confirmed", _app.getCmdrInvitedConfirmed(), rowInvitedConfirmed);
    displayInstancedList("Unconfirmed", _app.getCmdrInvitedUnconfirmed(), rowInvitedUnconfirmed);

    ImGui::EndChild();
}


void GUI::displayToInviteList(
    const char* title_name,
    const std::vector<std::string>& list,
    const ImU32& bgColor)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

    if (ImGui::BeginTable(title_name, 2, flags)) {
        std::string countText = title_name + std::string(" (") + std::to_string(list.size()) + ")";

        ImGui::TableSetupColumn(countText.c_str(), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        uint32_t uid = 0;

        ImGui::PushFont(_fontEurocaps);

        for (const std::string& cmdr : list) {
            // Ensures height stay consistent regardless if edit buttons are displayed
            const float rowHeight = ImGui::GetFrameHeight();

            ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, bgColor);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
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
}


void GUI::displayInstancedList(
    const char* title_name,
    const std::vector<std::string>& list,
    const ImU32& bgColor)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;
    if (ImGui::BeginTable(title_name, 2, flags)) {
        std::string countText = title_name + std::string(" (") + std::to_string(list.size()) + ")";

        ImGui::TableSetupColumn("");
        ImGui::TableSetupColumn(countText.c_str(), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        uint32_t uid = 0;
        const std::vector<std::string>& cmdrInvited = list;

        ImGui::PushFont(_fontEurocaps);

        for (const std::string& cmdr : cmdrInvited) {
            // Ensures height stay consistent regardless if edit buttons are displayed
            const float rowHeight = ImGui::GetFrameHeight();

            ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, bgColor);

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
            ImGui::AlignTextToFramePadding();
            ImGui::Text(cmdr.c_str());
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }
}


void GUI::showConfirmationMessages()
{
    static char inputName[255] = { 0 };

    // Close confirmation if needed
    if (_mainWindow->closeRequested() && _mainWindow->isCloseConfirmationRequired()) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("Confirm Exit");

        const std::string wingingProgressStr =
            "You currently have invited commanders.\n"
            "Closing the application will cause you to lose the current winging progress.\n\n";

        const std::string unsavedDocStr = "Your have unsaved commanders in the list.\n\n";

        std::string messageStr;

        if (_app.getCmdrInvitedConfirmed().size() + _app.getCmdrInvitedUnconfirmed().size() > 0) {
            messageStr += wingingProgressStr;
        }

        if (_app.wasEditedSinceLastSave()) {
            messageStr += unsavedDocStr;
        }

        messageStr += 
            "This action cannot be undone.\n"
            "Are you sure you want to exit the program ?\n\n";

        if (ImGui::BeginPopupModal("Confirm Exit", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(messageStr.c_str());

            ImVec2 textSize = ImGui::CalcTextSize("Confirm Exit");
            float buttonWidth = textSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;

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

            ImVec2 textSize = ImGui::CalcTextSize("Cancel");
            float buttonWidth = textSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;
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


void GUI::openNewCommanderListDialog()
{
    _mainWindow->openFileDialog(
        "Open new commander list",
        {
            { "Text file",  "txt" },
            { "CSV file",   "csv" }
        },
        this, GUI::loadCommanderList
    );
}


void GUI::appendCommanderListDialog()
{
    _mainWindow->openFileDialog(
        "Append commander list",
        {
            { "Text file",  "txt" },
            { "CSV file",   "csv" }
        },
        this, GUI::appendCommanderList
    );
}


void GUI::saveCommanderListDialog()
{
    _mainWindow->saveFileDialog(
        "Save commander list",
        {
            { "Text file",  "txt" }
        },
        this, GUI::exportCommanderList
    );
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