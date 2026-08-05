#include <windows.h>

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
        // Hooking work must not run on the loader lock - spawn a thread here
        // once there is something to do.
        DisableThreadLibraryCalls(module);
    }

    return TRUE;
}
