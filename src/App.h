#pragma once

#include "watchers/JournalWatcher.h"

#include <array>
#include <filesystem>
#include <map>
#include <string>


class App : public JournalListener
{
public:
    struct Status {
        enum CurrStatus {
            NeedsInvite_Offline,
            NeedsInvite_Online,
            AlreadyWingedActive,
            AlreadyWingedUnconfirmed,
            N_STATUS
        };

        CurrStatus status;
        bool manualyMovedToWing = false;
    };

    App();
    ~App();

	void monitorUserProfile(const std::filesystem::path& userProfile);

    void loadCommanderList(const std::filesystem::path& pathList);
    void appendCommanderList(const std::filesystem::path& pathList);
    void exportCommanderList(const std::filesystem::path& path);
    bool wasEditedSinceLastSave() const { return _wasEdited; }

    void addCommander(std::string commanderName, bool refresh = true);
    void removeCommander(std::string commanderName);

    const std::map<std::string, Status>& getCmdrList() const { return _cmdrList; }

    const std::vector<std::string>& getCmdrNeedInviteOnline() const { return _cmdrListFiltered[Status::NeedsInvite_Online]; }
    const std::vector<std::string>& getCmdrNeedInviteOffline() const { return _cmdrListFiltered[Status::NeedsInvite_Offline]; }
    const std::vector<std::string>& getCmdrInvitedConfirmed() const { return _cmdrListFiltered[Status::AlreadyWingedActive]; }
    const std::vector<std::string>& getCmdrInvitedUnconfirmed() const { return _cmdrListFiltered[Status::AlreadyWingedUnconfirmed]; }

    const std::map<std::string, bool>& getOnlineFriends() const { return _friendsOnlineTracker; }

    void setCmdrStatusInvited(const std::string& cmdrName);
    void setCmdrStatusWaiting(const std::string& cmdrName);

    void onJournalEvent(const std::string& event, const std::string& journalEntry);

private:
    void handleFriendEvent(const std::string& journalEntry);
    void handleWingEvent(const std::string& journalEntry);
    void handleReceiveTextEvent(const std::string& journalEntry);
    void handleShutdownEvent();
    void handleFileheaderEvent(const std::string& journalEntry);

    void refreshSortedLists();

    void toCmdrName(std::string& s);

private:
    std::map<std::string, Status> _cmdrList;

    // Sorted lists: duplicated of _cmdrList
    std::array<std::vector<std::string>, Status::N_STATUS> _cmdrListFiltered;

    std::map<std::string, bool> _friendsOnlineTracker;

    JournalWatcher _journalWatcher;

    bool _wasEdited = false;
};