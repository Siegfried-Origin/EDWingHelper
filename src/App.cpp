#include "App.h"

#include <json.hpp>
#include <fstream>

App::App(const std::filesystem::path& execPath)
{ }


App::~App()
{ }


void App::loadCommanderList(const std::filesystem::path& pathList)
{
    // TODO: support for update of current status, i.e., the already invited commanders
    //       shall get the appropriate flag
    _cmdrList.clear();

    std::ifstream in(pathList);
    if (!in.is_open()) {
        // Silently fail in release, assert in debug
        assert(0);
        return;
    }

    auto ltrim = [](std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));
    };
    auto rtrim = [](std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
            [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    };
    auto trim = [&](std::string &s) {
        ltrim(s);
        rtrim(s);
    };

    std::string cmdrName;
    while (std::getline(in, cmdrName)) {
        trim(cmdrName);

        if (cmdrName.empty()) {
            continue;
        }

        toCmdrName(cmdrName);

        Status status = NeedsInvite_Offline;

        // Check from previous recorded events if the commander is online
        auto it = _friendsOnlineTracker.find(cmdrName);

        if (it != _friendsOnlineTracker.end() &&
            _friendsOnlineTracker[cmdrName]) {
            status = NeedsInvite_Online;
        }

        _cmdrList.emplace(cmdrName, status);
    }
}


void App::handleFriendEvent(const std::string& journalEvent)
{
    const nlohmann::json json = nlohmann::json::parse(journalEvent);

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
            it->second = NeedsInvite_Online;
        }
        else if (status == "Online") {
            if (it->second == Invited) {
                // This shall not happen, crash in debug run
                assert(0);
            }

            it->second = NeedsInvite_Online;
        }
        else if (status == "Offline") {
            it->second = NeedsInvite_Offline;
        }
    }

    // TODO: same as before, check for the behavior of "Added" event
    _friendsOnlineTracker.insert_or_assign(cmdrName, status == "Online" || status == "Added");
}


void App::handleWingEvent(const std::string& journalEvent)
{
    const nlohmann::json json = nlohmann::json::parse(journalEvent);

    if (!json.contains("event") ||
        !json.contains("Name")) {
        // Silently ignore when not in debug mode, this shall not happen
        assert(0);
        return;
    }

    const std::string& event = json["event"];
    std::string cmdrName = json["Name"];
    toCmdrName(cmdrName);

    auto it = _cmdrList.find(cmdrName);

    if (it != _cmdrList.end()) {
        if (event == "WingAdd") {

            it->second = Invited;
        }
    }

    // WingLeave
    // { "timestamp":"2026-03-08T20:04:06Z", "event":"WingLeave" }

    // TODO: we want to make all others commanders as JoinedWing.
    //       Also, check if the even list the inviting commander as well
    // WingJoin
    // { "timestamp":"2026-03-08T20:04:41Z", "event":"WingJoin", "Others":[] }
}


void App::handleShutdownEvent()
{
    _cmdrList.clear();
    _friendsOnlineTracker.clear();
}


void App::toCmdrName(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        }
    );
}