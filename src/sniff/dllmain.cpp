#include <windows.h>

#include <cstdio>
#include <vector>

// The GUI client has no console
static void OpenLogConsole() {
    if (!AllocConsole()) {
        return;
    }
    SetConsoleTitleA("mollen-sniffer");
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
}

// Build number from the exe's version resource
static DWORD HostBuild() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(nullptr, path, MAX_PATH) == 0) {
        return 0;
    }

    DWORD ignored = 0;
    DWORD size = GetFileVersionInfoSizeA(path, &ignored);
    if (size == 0) {
        return 0;
    }

    std::vector<unsigned char> buf(size);
    if (!GetFileVersionInfoA(path, 0, size, buf.data())) {
        return 0;
    }

    VS_FIXEDFILEINFO* info = nullptr;
    UINT len = 0;
    if (!VerQueryValueA(buf.data(), "\\", reinterpret_cast<void**>(&info), &len)
        || info == nullptr) {
        return 0;
    }

    // low word of LS = build
    return LOWORD(info->dwFileVersionLS);
}

static DWORD WINAPI Worker(LPVOID) {
    OpenLogConsole();

    DWORD build = HostBuild();
    if (build != 0) {
        std::printf("mollen-sniff: attached to build %lu\n", build);
    } else {
        std::printf("mollen-sniff: attached (build unknown)\n");
    }
    std::fflush(stdout);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
        // Minimal under the loader lock: hand off to a worker thread.
        DisableThreadLibraryCalls(module);
        HANDLE t = CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr);
        if (t) {
            CloseHandle(t);
        }
    }

    return TRUE;
}
