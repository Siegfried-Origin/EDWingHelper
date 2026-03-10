#pragma once

#include <filesystem>
#include <map>
#include <string>

class App
{
public:
    enum Status {
        NeedsInvite_Online,
        NeedsInvite_Offline,
        Invited
    };

    App(const std::filesystem::path& execPath);

    ~App();

    void loadCommanderList(const std::filesystem::path& pathList);

private:
    void handleFriendEvent(const std::string& journalEntry);
    void handleWingEvent(const std::string& journalEntry);
    void handleShutdownEvent();

    void toCmdrName(std::string& s);

private:
    std::map<std::string, Status> _cmdrList;

    std::map<std::string, bool> _friendsOnlineTracker;
};