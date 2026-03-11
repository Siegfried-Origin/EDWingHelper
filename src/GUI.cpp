#include "GUI.h"

#include "fonts/eurocaps.h"
#include "fonts/eurostile.h"
#include "fonts/icons.h"
#include "fonts/IconsMaterialDesign.h"


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
    static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
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

    fontEurocaps = io.Fonts->AddFontFromMemoryCompressedTTF(
        EUROCAPS_compressed_data,
        EUROCAPS_compressed_size,
        eurocapsFontSize
    );

    io.Fonts->AddFontFromMemoryCompressedTTF(
        Icons_compressed_data,
        Icons_compressed_size,
        iconFontSize,
        &cfgIcons,
        icons_ranges
    );

    // Eurostile
    fontEurostile = io.Fonts->AddFontFromMemoryCompressedTTF(
        Eurostile_compressed_data,
        Eurostile_compressed_size,
        fontSize
    );

    io.Fonts->AddFontFromMemoryCompressedTTF(
        Icons_compressed_data,
        Icons_compressed_size,
        iconFontSize,
        &cfgIcons,
        icons_ranges
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
            ImGui::PushFont(fontEurostile);

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
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open new commander list...")) {
                _mainWindow->openCommanderListFileDialog(this, GUI::loadCommanderList);
            }

            if (ImGui::MenuItem("Append commander list...")) {
                _mainWindow->openCommanderListFileDialog(this, GUI::appendCommanderList);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}


void GUI::showCommanderLists()
{
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

    // ------------------------------------------------------------------------
    // Need invite
    // ------------------------------------------------------------------------

    ImGui::BeginChild("Need Invite", ImVec2(512, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
    ImGui::Text("Need invite");
    ImGui::Separator();

    // Online
    if (ImGui::BeginTable("Online", 2, flags)) {
        ImGui::TableSetupColumn("Online", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        uint32_t uid = 0;

        const std::vector<std::string>& cmdrNeedsInviteOnline = _app.getCmdrNeedInviteOnline();

        ImGui::PushFont(fontEurocaps);

        for (const std::string& cmdr : cmdrNeedsInviteOnline) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(cmdr.c_str());

            ImGui::TableNextColumn();

            ImGui::PushID(uid++);

            if (ImGui::Button(ICON_MD_ARROW_FORWARD)) {
                _app.setCmdrStatus(cmdr, App::Invited);
            }

            if (ImGui::IsItemHovered()) {
                ImGui::PopFont();
                ImGui::SetTooltip("Manualy move to the invited list");
                ImGui::PushFont(fontEurocaps);
            }
            ImGui::PopID();
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }

    // Offline
    if (ImGui::BeginTable("Offline", 2, flags)) {
        ImGui::TableSetupColumn("Offline", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        const std::vector<std::string>& cmdrNeedsInviteOffline = _app.getCmdrNeedInviteOffline();

        ImGui::PushFont(fontEurocaps);

        for (const std::string& cmdr : cmdrNeedsInviteOffline) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(cmdr.c_str());

            ImGui::TableNextColumn();
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }

    ImGui::EndChild();

    // ------------------------------------------------------------------------
    // Already invited
    // ------------------------------------------------------------------------

    ImGui::SameLine();
    ImGui::BeginChild("Invited", ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGui::Text("Already invited");
    ImGui::Separator();

    if (ImGui::BeginTable("In Wing", 2, flags)) {
        ImGui::TableSetupColumn("");
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        uint32_t uid = 0;
        const std::vector<std::string>& cmdrInvited = _app.getCmdrInvited();

        ImGui::PushFont(fontEurocaps);

        for (const std::string& cmdr : cmdrInvited) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(uid++);
            
            if (ImGui::Button(ICON_MD_ARROW_BACK)) {
                _app.setCmdrStatus(cmdr, App::NeedsInvite_Online);
            }

            if (ImGui::IsItemHovered()) {
                ImGui::PopFont();
                ImGui::SetTooltip("Manualy remove from the invited list");
                ImGui::PushFont(fontEurocaps);
            }
            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::Text(cmdr.c_str());
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }

    ImGui::EndChild();
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