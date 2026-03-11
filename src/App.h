#pragma once

#include "watchers/JournalWatcher.h"

#include <array>
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
    void appendCommanderList(const std::filesystem::path& pathList);
    void exportCommanderList(const std::filesystem::path& path);

    void addCommander(std::string commanderName, bool refresh = true);
    void removeCommander(std::string commanderName);

    void onJournalEvent(const std::string& event, const std::string& journalEntry);

    const std::map<std::string, Status>& getCmdrList() const { return _cmdrList; }
    
    const std::vector<std::string>& getCmdrNeedInviteOnline() const { return _cmdrListFiltered[NeedsInvite_Online]; }
    const std::vector<std::string>& getCmdrNeedInviteOffline() const { return _cmdrListFiltered[NeedsInvite_Offline]; }
    const std::vector<std::string>& getCmdrInvited() const { return _cmdrListFiltered[Invited]; }

    const std::map<std::string, bool>& getOnlineFriends() const { return _friendsOnlineTracker; }

    void setCmdrStatusInvited(const std::string& cmdrName);
    void setCmdrStatusWaiting(const std::string& cmdrName);

private:
    void handleFriendEvent(const std::string& journalEntry);
    void handleWingEvent(const std::string& journalEntry);
    void handleShutdownEvent();
    void handleFileheaderEvent(const std::string& journalEntry);

    void refreshSortedLists();

    void toCmdrName(std::string& s);

#ifdef _WIN32
    void fileWatcherThread(HANDLE hStop);
#else
    void fileWatcherThread();
#endif

private:
    std::map<std::string, Status> _cmdrList;

    // Sorted lists: duplicated of _cmdrList
    std::array<std::vector<std::string>, App::N_STATUS> _cmdrListFiltered;

    std::map<std::string, bool> _friendsOnlineTracker;

    JournalWatcher _journalWatcher;
    std::thread _watcherThread;

#ifdef _WIN32
    HANDLE _hStop;
#else
    bool _hStop = false;
#endif
};