#include "injector.h"

#include <cstdio>

// Absolute path so LoadLibrary doesn't search the client's directory instead.
std::string PayloadPath() {
    char exe[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, exe, MAX_PATH);
    std::string path(exe, n);
    size_t slash = path.find_last_of("\\/");
    if (slash != std::string::npos) {
        path.resize(slash + 1);
    }
    path += "mollen-sniff.dll";
    return path;
}

bool InjectDll(DWORD pid, const std::string& dll_path) {
    HANDLE proc = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
            PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, pid);
    if (!proc) {
        std::fprintf(stderr, "OpenProcess failed (%lu)\n", GetLastError());
        return false;
    }

    SIZE_T size = dll_path.size() + 1;
    void* remote = VirtualAllocEx(proc, nullptr, size, MEM_COMMIT | MEM_RESERVE,
                                  PAGE_READWRITE);
    if (!remote) {
        std::fprintf(stderr, "VirtualAllocEx failed (%lu)\n", GetLastError());
        CloseHandle(proc);
        return false;
    }

    bool ok = false;
    if (WriteProcessMemory(proc, remote, dll_path.c_str(), size, nullptr)) {
        // kernel32 is at the same address in every process this boot, so this
        // LoadLibraryA pointer is valid inside the target.
        auto load = reinterpret_cast<LPTHREAD_START_ROUTINE>(
            GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));

        HANDLE thread = load ? CreateRemoteThread(proc, nullptr, 0, load, remote, 0, nullptr) : nullptr;
        if (thread) {
            WaitForSingleObject(thread, INFINITE);
            // Exit code = LoadLibraryA's return (HMODULE), 0 on failure.
            DWORD loaded = 0;
            GetExitCodeThread(thread, &loaded);
            CloseHandle(thread);
            ok = (loaded != 0);
            if (!ok) {
                std::fprintf(stderr, "LoadLibrary in target returned NULL\n");
            }
        } else {
            std::fprintf(stderr, "CreateRemoteThread failed (%lu)\n", GetLastError());
        }
    } else {
        std::fprintf(stderr, "WriteProcessMemory failed (%lu)\n", GetLastError());
    }

    VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
    CloseHandle(proc);
    return ok;
}
