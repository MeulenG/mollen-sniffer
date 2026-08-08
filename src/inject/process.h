#ifndef MOLLEN_SNIFFER_PROCESS_H
#define MOLLEN_SNIFFER_PROCESS_H

#include <windows.h>

#include <string>
#include <vector>

struct WowProcess {
    DWORD       pid;
    std::string window_title;   // may be empty
};

std::vector<WowProcess> FindRunningProcesses();

// Prompt to choose among multiple clients; returns the index, or -1 if invalid.
int HandleMultipleInstances(const std::vector<WowProcess>& procs);

#endif // MOLLEN_SNIFFER_PROCESS_H
