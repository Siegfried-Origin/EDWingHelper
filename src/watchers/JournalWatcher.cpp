#include "JournalWatcher.h"

#include "../util/EliteFileUtil.h"

#include <iostream>
#include <json.hpp>


JournalWatcher::JournalWatcher()
{
    _stopWatcherThread = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}


JournalWatcher::~JournalWatcher()
{
    _stopForceUpdate = true;

    if (_forcedUpdateThread.joinable()) {
        _forcedUpdateThread.join();
    }

#ifdef _WIN32
    SetEvent(_stopWatcherThread);
#else
    _stopWatcherThread = true;
#endif

    if (_watcherThread.joinable()) {
        _watcherThread.join();
    }

#ifdef _WIN32
    CloseHandle(_stopWatcherThread);
#endif
}


void JournalWatcher::addListener(JournalListener* listener)
{
    _listeners.push_back(listener);
}


void JournalWatcher::start(const std::filesystem::path& userProfile)
{
    const std::filesystem::path& latestJournal = EliteFileUtil::getLatestJournal(userProfile);
    
    if (!std::filesystem::exists(latestJournal)) {
        throw std::runtime_error("Cannot find journal file: " + latestJournal.string() + ". Did you lanch the game previously?");
    }

    if (latestJournal != _currJournalPath) {
        // The observed journal changed
        _currJournalFile.close();
        _currJournalFile = std::ifstream(latestJournal);
        _currJournalPath = latestJournal;

        std::wcout << L"[INFO  ] Monitoring: " << _currJournalPath << std::endl;
    }

    // Prime the listeners with existing entries
    std::string line;

    while (std::getline(_currJournalFile, line)) {
        auto j = nlohmann::json::parse(line);

        for (JournalListener* listener : _listeners) {
            listener->onJournalEvent(j["event"], line);
        }
    }

    _stopForceUpdate = false;
    _forcedUpdateThread = std::thread(&JournalWatcher::forcedUpdate, this);

#ifdef _WIN32
    _watcherThread = std::thread(&JournalWatcher::fileWatcherThread, this, userProfile, _stopWatcherThread);
#else
    _stopWatcherThread = false;
    _watcherThread = std::thread(&JournalWatcher::fileWatcherThread, this, userProfile);
#endif
}


void JournalWatcher::update(const std::filesystem::path& filename)
{
    if (filename != _currJournalPath) {
        // The observed journal changed
        _currJournalFile.close();
        _currJournalFile = std::ifstream(filename);
        _currJournalPath = filename;

        std::wcout << L"[INFO  ] Monitoring: " << _currJournalPath << std::endl;
    }

    std::string line;

    while (std::getline(_currJournalFile, line)) {
        auto j = nlohmann::json::parse(line);

        for (JournalListener* listener : _listeners) {
            listener->onJournalEvent(j["event"], line);
        }
    }

    // Clear EOF flag
    _currJournalFile.clear();
}


void JournalWatcher::forcedUpdate() {
    // We need to regularly load the file to avoid miss in the WIN32 monitoring
    // in case the file was not flushed to the disk.
    while (!_stopForceUpdate) {
        std::wifstream file(_currJournalPath, std::ios::in | std::ios::binary);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


#ifdef _WIN32
void JournalWatcher::fileWatcherThread(const std::filesystem::path userProfile, HANDLE hStop)
{
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
                    update(userProfile / filename);
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
void JournalWatcher::fileWatcherThread(const std::filesystem::path userProfile)
{
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

    while (!_stopWatcherThread) {
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
                    update(fullpath);
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