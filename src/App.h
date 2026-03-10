#pragma once

#include "watchers/JournalWatcher.h"

#include <filesystem>
#include <map>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif


class App : public JournalListener
{
public:
    enum Status {
        NeedsInvite_Online,
        NeedsInvite_Offline,
        Invited,
        N_STATUS
    };

    App(const std::filesystem::path& execPath);

    ~App();

    void loadCommanderList(const std::filesystem::path& pathList);

    void onJournalEvent(const std::string& event, const std::string& journalEntry);

    const std::map<std::string, Status>& getCmdrList() const { return _cmdrList; }
    const std::map<std::string, bool>& getOnlineFriends() const { return _friendsOnlineTracker; }

private:
    void handleFriendEvent(const std::string& journalEntry);
    void handleWingEvent(const std::string& journalEntry);
    void handleShutdownEvent();
    void handleFileheaderEvent(const std::string& journalEntry);

    void toCmdrName(std::string& s);

#ifdef _WIN32
    void fileWatcherThread(HANDLE hStop);
#else
    void fileWatcherThread();
#endif

private:
    std::map<std::string, Status> _cmdrList;

    std::map<std::string, bool> _friendsOnlineTracker;

    JournalWatcher _journalWatcher;
    std::thread _watcherThread;

#ifdef _WIN32
    HANDLE _hStop;
#else
    bool _hStop = false;
#endif
};