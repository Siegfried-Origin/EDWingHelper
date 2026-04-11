#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <thread>
#include <atomic>


#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/inotify.h>
#  include <unistd.h>
#  include <limits.h>
#endif


class JournalListener
{
public:
    virtual void onJournalEvent(const std::string& event, const std::string& jounralEntry) = 0;
};


class JournalWatcher
{
public:
    JournalWatcher();
    virtual ~JournalWatcher();

    void addListener(JournalListener* listener);

    void start(const std::filesystem::path& userProfile);

private:
    void update(const std::filesystem::path& filename);

    void forcedUpdate();

#ifdef _WIN32
    void fileWatcherThread(const std::filesystem::path userProfile, HANDLE hStop);
#else
    void fileWatcherThread(const std::filesystem::path userProfile);
#endif

private:
    std::filesystem::path _currJournalPath;
    std::ifstream _currJournalFile;
    std::vector<JournalListener*> _listeners;

    std::atomic<bool> _stopForceUpdate;
    std::thread _forcedUpdateThread;
    std::thread _watcherThread;

#ifdef _WIN32
    HANDLE _stopWatcherThread;
#else
    bool _hStop = false;
#endif
};