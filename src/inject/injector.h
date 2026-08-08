#ifndef MOLLEN_SNIFFER_INJECTOR_H
#define MOLLEN_SNIFFER_INJECTOR_H

#include <windows.h>

#include <string>

std::string PayloadPath();

// Inject dll_path via CreateRemoteThread(LoadLibraryA). True if it loaded.
bool InjectDll(DWORD pid, const std::string& dll_path);

#endif // MOLLEN_SNIFFER_INJECTOR_H
