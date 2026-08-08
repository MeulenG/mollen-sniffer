#include "injector.h"
#include "process.h"

#include <cstdio>
#include <string>
#include <vector>

int main() {
    std::vector<WowProcess> procs = FindRunningProcesses();
    if (procs.empty()) {
        std::fprintf(stderr, "No Wow.exe process found - is the client running?\n");
        return 1;
    }

    int index = 0;
    if (procs.size() > 1) {
        index = HandleMultipleInstances(procs);
        if (index < 0) {
            std::fprintf(stderr, "Invalid selection.\n");
            return 1;
        }
    }

    const WowProcess& target = procs[index];
    std::printf("Target: pid %lu  %s\n", target.pid, target.window_title.empty() ? "(no window title)" : target.window_title.c_str());

    std::string dll = PayloadPath();
    if (GetFileAttributesA(dll.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::fprintf(stderr, "Payload DLL not found next to injector: %s\n", dll.c_str());
        return 1;
    }

    if (!InjectDll(target.pid, dll)) {
        std::fprintf(stderr, "Injection failed.\n");
        return 1;
    }

    std::printf("Injected %s\n", dll.c_str());
    return 0;
}
