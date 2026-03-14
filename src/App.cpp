#include "App.h"

#include "util/EliteFileUtil.h"


#include <fstream>
#include <iostream>
#include <json.hpp>


App::App(const std::filesystem::path& execPath)
    : _journalWatcher(EliteFileUtil::getLatestJournal(EliteFileUtil::getUserProfile()))
{
    _journalWatcher.addListener(this);
    _journalWatcher.start();

#ifdef _WIN32
    _hStop = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    _watcherThread = std::thread(
        &App::fileWatcherThread,
        this,
        _hStop);
#else
    _hStop = false;
    _watcherThread = std::thread(
        &App::fileWatcherThread,
        this
    );
#endif
}


App::~App()
{
#ifdef _WIN32
    SetEvent(_hStop);
    _watcherThread.join();
    CloseHandle(_hStop);
#else
    _hStop = true;
    if (_watcherThread.joinable()) {
        _watcherThread.join();
    }
#endif
}


void App::loadCommanderList(const std::filesystem::path& pathList)
{
    _cmdrList.clear();
    appendCommanderList(pathList);
    _wasEdited = false;
}


void App::appendCommanderList(const std::filesystem::path& pathList)
{
    // TODO: support for update of current status, i.e., the already invited commanders
    //       shall get the appropriate flag

    std::ifstream in(pathList);

    if (!in.is_open()) {
        // Silently fail in release, assert in debug
        assert(0);
        return;
    }

    std::string cmdrName;

    while (std::getline(in, cmdrName)) {
        addCommander(cmdrName, false);
    }

    refreshSortedLists();
}


void App::addCommander(std::string commanderName, bool refresh)
{
    auto ltrim = [](std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));
        };
    auto rtrim = [](std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
            [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
        };
    auto trim = [&](std::string& s) {
        ltrim(s);
        rtrim(s);
        };

    trim(commanderName);

    if (!commanderName.empty()) {
        toCmdrName(commanderName);

        Status status = { Status::NeedsInvite_Offline, false };

        // Check from previous recorded events if the commander is online
        auto it = _friendsOnlineTracker.find(commanderName);

        if (it != _friendsOnlineTracker.end() &&
            _friendsOnlineTracker[commanderName]) {
            status = { Status::NeedsInvite_Online, false };
        }

        _cmdrList.emplace(commanderName, status);

        _wasEdited = true;
    }

    if (refresh) {
        refreshSortedLists();
    }
}


void App::removeCommander(std::string commanderName)
{
    toCmdrName(commanderName);
    const size_t elemRemoved = _cmdrList.erase(commanderName);
    _wasEdited = elemRemoved > 0;

    refreshSortedLists();
}


void App::exportCommanderList(const std::filesystem::path& path)
{
    std::ofstream out(path);

    for (const auto& it : _cmdrList) {
        out << it.first;
        out << std::endl;
    }

    _wasEdited = false;
}


void App::setCmdrStatusInvited(const std::string& cmdrName)
{
    auto it = _cmdrList.find(cmdrName);

    if (it != _cmdrList.end()) {
        it->second = { Status::AlreadyWingedUnconfirmed, true };
    }

    refreshSortedLists();
}


void App::setCmdrStatusWaiting(const std::string& cmdrName)
{
    auto it = _cmdrList.find(cmdrName);

    if (it != _cmdrList.end()) {
        auto itFriendList = _friendsOnlineTracker.find(cmdrName);

        if (itFriendList != _friendsOnlineTracker.end() &&
            _friendsOnlineTracker[cmdrName]) {
            it->second = { Status::NeedsInvite_Online, true };
        }
        else {
            it->second = { Status::NeedsInvite_Offline, true };
        }
    }

    refreshSortedLists();
}


void App::onJournalEvent(const std::string& event, const std::string& journalEntry)
{
    if (event == "Friends") {
        handleFriendEvent(journalEntry);
    }
    else if (event == "WingAdd" || event == "WingJoin" || event == "WingLeave") {
        handleWingEvent(journalEntry);
    }
    else if (event == "ReceiveText") {
        handleReceiveTextEvent(journalEntry);
    }
    else if (event == "Shutdown") {
        handleShutdownEvent();
    }
    else if (event == "Fileheader") {
        handleFileheaderEvent(journalEntry);
    }
}


void App::handleFriendEvent(const std::string& journalEntry)
{
    const nlohmann::json json = nlohmann::json::parse(journalEntry);

    if (!json.contains("event") || json["event"] != "Friends" ||
        !json.contains("Status") ||
        !json.contains("Name")) {
        // Silently ignore when not in debug mode, this shall not happen
        assert(0);
        return;
    }

    const std::string& status = json["Status"];
    std::string cmdrName = json["Name"];
    toCmdrName(cmdrName);

    auto it = _cmdrList.find(cmdrName);

    if (it != _cmdrList.end()) {
        if (status == "Added") {
            // TODO: check if this is triggered after accepting an invite
            //       or also when inviting a new - potentially offline -
            //       cmdr to our friend list
            it->second = { Status::NeedsInvite_Online, false };;
        }
        else if (status == "Online") {
            /*
            This is not reliable: sometimes we get two `Online` event
            without the matching `Offline` event...
            if (it->second.status == Status::AlreadyWingedActive ||
                it->second.status == Status::AlreadyWingedUnconfirmed) {
                // We move the commander to the "auto" section to then detect potential
                // future disconect
                if (it->second.manualyMovedToWing) {
                    it->second.manualyMovedToWing = false;
                }
                else {
                    // This commander experienced a disconnection durign the instance
                    // creation
                    it->second = { Status::NeedsInvite_Online, false };
                }
            }
            else */ if (it->second.status == Status::NeedsInvite_Offline) {
                // This commander became online
                it->second = { Status::NeedsInvite_Online, false };
            }
        }
        else if (status == "Offline") {
            it->second = { Status::NeedsInvite_Offline, false };
        }
    }

    // TODO: same as before, check for the behavior of "Added" event
    _friendsOnlineTracker.insert_or_assign(cmdrName, status == "Online" || status == "Added");

    refreshSortedLists();
}


void App::handleWingEvent(const std::string& journalEntry)
{
    const nlohmann::json json = nlohmann::json::parse(journalEntry);

    if (!json.contains("event")) {
        // Silently ignore when not in debug mode, this shall not happen
        assert(0);
        return;
    }

    const std::string& event = json["event"];

    if (event == "WingAdd") {
        if (!json.contains("Name")) {
            // Silently ignore when not in debug mode, this shall not happen
            assert(0);
            return;
        }

        std::string cmdrName = json["Name"];
        toCmdrName(cmdrName);

        auto it = _cmdrList.find(cmdrName);

        if (it != _cmdrList.end()) {
            if (event == "WingAdd") {
                if (it->second.status != Status::AlreadyWingedActive) {
                    it->second.status = Status::AlreadyWingedUnconfirmed;
                }

                it->second.manualyMovedToWing = false;
            }
        }
    }
    else if (event == "WingJoin") {
        if (!json.contains("Others")) {
            assert(0);
            return;
        }

        for (const auto& other : json["Others"]) {
            std::string cmdrName = other.get<std::string>();
            toCmdrName(cmdrName);

            auto it = _cmdrList.find(cmdrName);
            if (it != _cmdrList.end()) {
                if (it->second.status != Status::AlreadyWingedActive) {
                    it->second.status = Status::AlreadyWingedUnconfirmed;
                }

                it->second.manualyMovedToWing = false;
            }
        }
    }

    refreshSortedLists();
}


void App::handleReceiveTextEvent(const std::string& journalEntry)
{
    const nlohmann::json json = nlohmann::json::parse(journalEntry);

    if (!json.contains("event") || json["event"] != "ReceiveText" ||
        !json.contains("From") ||
        !json.contains("Channel")) {
        // Silently ignore when not in debug mode, this shall not happen
        assert(0);
        return;
    }

    const std::string& from = json["From"];
    const std::string& channel = json["Channel"];

    if (channel == "local") {
        std::string cmdrName = from;
        toCmdrName(cmdrName);
        auto it = _cmdrList.find(cmdrName);

        if (it != _cmdrList.end()) {
            switch (it->second.status) {
                case Status::NeedsInvite_Online:
                case Status::AlreadyWingedActive:
                case Status::AlreadyWingedUnconfirmed:
                // Even offline can be detected in instance in case they are not in
                // the commander friend list
                case Status::NeedsInvite_Offline:
                    it->second = { Status::AlreadyWingedActive, false };
                    break;
                case Status::N_STATUS:
                    // That shall not happen
                    assert(0);
                    break;
            }
        }
    }

    refreshSortedLists();
}


void App::handleShutdownEvent()
{
    _friendsOnlineTracker.clear();

    for (auto& cmdr: _cmdrList) {
        // Keeps the already instanced commanders as it
        if (cmdr.second.status == Status::NeedsInvite_Online) {
            cmdr.second.status = Status::NeedsInvite_Offline;
        }

        // Now all winged commander must be switched to "manually winged"
        // to prevent moving them back to the NeedsInvite section after
        // an online event
        if (cmdr.second.status == Status::AlreadyWingedActive ||
            cmdr.second.status == Status::AlreadyWingedUnconfirmed) {
            cmdr.second = { Status::AlreadyWingedUnconfirmed, true };
        }
    }

    refreshSortedLists();
}


void App::handleFileheaderEvent(const std::string& journalEntry)
{
    const nlohmann::json json = nlohmann::json::parse(journalEntry);

    if (!json.contains("event") || json["event"] != "Fileheader" ||
        !json.contains("part")) {
        // Silently ignore when not in debug mode, this shall not happen
        assert(0);
        return;
    }

    if (json["part"] == 1) {
        _friendsOnlineTracker.clear();
    }

    for (auto& cmdr : _cmdrList) {
        // We are not sure if the NeedInvite commanders are still online.
        // We will wait for the Friend Online event 
        if (cmdr.second.status == Status::NeedsInvite_Online) {
            cmdr.second.status = Status::NeedsInvite_Offline;
        }

        // Now all winged commander must be switched to "manually winged"
        // to prevent moving them back to the NeedsInvite section after
        // an online event
        if (cmdr.second.status == Status::AlreadyWingedActive ||
            cmdr.second.status == Status::AlreadyWingedUnconfirmed) {
            cmdr.second = { Status::AlreadyWingedUnconfirmed, true };
        }
    }

    refreshSortedLists();
}


void App::refreshSortedLists()
{
    for (std::vector<std::string>& list : _cmdrListFiltered) {
        list.clear();
    }

    for (auto const& [cmdr, status] : _cmdrList) {
        _cmdrListFiltered[status.status].push_back(cmdr);
    }
}


void App::toCmdrName(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        }
    );
}


#ifdef _WIN32
void App::fileWatcherThread(HANDLE hStop)
{
    const std::filesystem::path userProfile = EliteFileUtil::getUserProfile();

    HANDLE hDir = CreateFileW(
        userProfile.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERR   ] Cannot open status folder." << std::endl;
        return;
    }

    OVERLAPPED ov{};
    ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (ov.hEvent == NULL) {
        std::cerr << "[ERR   ] CreateEvent failed: " << GetLastError() << std::endl;
        CloseHandle(hDir);
        return;
    }

    char buffer[1024] = { 0 };
    DWORD bytesReturned = 0;

    auto postRead = [&]() {
        ResetEvent(ov.hEvent);

        BOOL ok = ReadDirectoryChangesW(
            hDir,
            buffer,
            sizeof(buffer),
            FALSE,
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
            nullptr,
            &ov,
            nullptr
        );

        if (!ok && GetLastError() != ERROR_IO_PENDING) {
            std::cerr << "[ERR   ] ReadDirectoryChangesW failed: " << GetLastError() << std::endl;
        }
        };

    postRead();

    HANDLE handles[2] = { ov.hEvent, hStop };

    while (true) {
        DWORD w = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        if (w == WAIT_OBJECT_0) {
            // Read completed
            DWORD bytes = 0;
            if (!GetOverlappedResult(hDir, &ov, &bytes, FALSE)) {
                DWORD err = GetLastError();

                if (err == ERROR_OPERATION_ABORTED) {
                    // CancelIoEx called
                    break;
                }
                else {
                    // Try restarting the observer
                    std::cerr << "[ERR   ] GetOverlappedResult failed. err=" << err << std::endl;
                    postRead();
                    continue;
                }
            }

            // Check which files changed
            BYTE* ptr = reinterpret_cast<BYTE*>(buffer);

            while (true) {
                auto* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(ptr);
                std::wstring filename(fni->FileName, fni->FileNameLength / sizeof(WCHAR));

                if (EliteFileUtil::isJournalFile(filename)) {
                    _journalWatcher.update(userProfile / filename);
                }
                else {
                    std::wcout << L"[INFO  ] Ignoring file change: " << filename << std::endl;
                }

                if (fni->NextEntryOffset == 0) break;
                ptr += fni->NextEntryOffset;
            }

            // Restart the observer
            postRead();
        }
        else if (w == WAIT_OBJECT_0 + 1) {
            // Request to stop
            CancelIoEx(hDir, &ov);
            WaitForSingleObject(ov.hEvent, INFINITE);
            break;
        }
        else {
            std::cerr << "[ERR   ] WaitForMultipleObjects failed. err=" << GetLastError() << std::endl;
            break;
        }
    }

    CloseHandle(ov.hEvent);
    CloseHandle(hDir);
}
#else
void App::fileWatcherThread()
{
    const std::filesystem::path userProfile = EliteFileUtil::getUserProfile();

    const int inotifyFd = inotify_init1(IN_NONBLOCK);

    if (inotifyFd < 0) {
        std::cerr << "[ERR   ] inotify_init1 failed." << std::endl;
        return;
    }

    int watchFd = inotify_add_watch(inotifyFd, userProfile.c_str(), IN_MODIFY);

    if (watchFd < 0) {
        std::cerr << "[ERR   ] inotify_add_watch failed on: " << userProfile << std::endl;
        close(inotifyFd);
        return;
    }

    constexpr size_t bufSize = 1024 * (sizeof(struct inotify_event) + NAME_MAX + 1);
    char buffer[bufSize];

    while (!_hStop) {
        const int length = read(inotifyFd, buffer, bufSize);

        if (length < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            std::cerr << "[ERR   ] inotify read failed." << std::endl;
            break;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);

            if (event->len > 0 && (event->mask & IN_MODIFY)) {
                std::string filename(event->name);
                std::filesystem::path fullpath = userProfile / filename;

                if (EliteFileUtil::isJournalFile(filename)) {
                    _journalWatcher.update(fullpath);
                }
                else {
                    std::cout << "[INFO  ] Ignored file change: " << filename << std::endl;
                }
            }

            i += sizeof(struct inotify_event) + event->len;
        }
    }

    inotify_rm_watch(inotifyFd, watchFd);
    close(inotifyFd);
}
#endif
