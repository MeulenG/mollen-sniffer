#include "process.h"

#include <tlhelp32.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

namespace {

// matched case-insensitively
constexpr const char* kTargetExe = "Wow.exe";

// First visible window title per PID
BOOL CALLBACK CollectTitle(HWND hwnd, LPARAM param) {
    auto* titles = reinterpret_cast<std::map<DWORD, std::string>*>(param);

    if (!IsWindowVisible(hwnd)) {
        return TRUE;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0 || titles->count(pid)) {
        return TRUE;
    }

    char buf[256];
    int len = GetWindowTextA(hwnd, buf, sizeof(buf));
    if (len > 0) {
        (*titles)[pid] = std::string(buf, len);
    }
    return TRUE;
}

} // namespace

std::vector<WowProcess> FindRunningProcesses() {
    std::vector<WowProcess> found;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return found;
    }

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);
    if (Process32First(snap, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, kTargetExe) == 0) {
                found.push_back({entry.th32ProcessID, {}});
            }
        } while (Process32Next(snap, &entry));
    }
    CloseHandle(snap);

    if (!found.empty()) {
        std::map<DWORD, std::string> titles;
        EnumWindows(CollectTitle, reinterpret_cast<LPARAM>(&titles));
        for (auto& p : found) {
            auto it = titles.find(p.pid);
            if (it != titles.end()) {
                p.window_title = it->second;
            }
        }
    }

    return found;
}

int HandleMultipleInstances(const std::vector<WowProcess>& procs) {
    std::printf("Multiple %s processes found:\n", kTargetExe);
    for (size_t i = 0; i < procs.size(); ++i) {
        const std::string& title = procs[i].window_title;
        std::printf("  [%zu] pid %lu  %s\n", i, procs[i].pid,
                    title.empty() ? "(no window title)" : title.c_str());
    }
    std::printf("Select [0-%zu]: ", procs.size() - 1);

    char line[32];
    if (!std::fgets(line, sizeof(line), stdin)) {
        return -1;
    }
    char* end = nullptr;
    long choice = std::strtol(line, &end, 10);
    if (end == line || choice < 0 || choice >= static_cast<long>(procs.size())) {
        return -1;
    }
    return static_cast<int>(choice);
}
