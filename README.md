# mollen-sniffer

Packet sniffer for the WoW client. `mollen-inject` loads `mollen-sniff.dll`
into the running client, which hooks the network layer and writes captures in
PKT 3.1 format - the same container WowPacketParser reads, so captures can be
cross-checked against a known-good parser.

Part of the mollen toolchain. Target: 3.3.5a (build 12340), against private
servers. No game assets or client code are included; bring your own client.

## Build

32-bit only, since the client is a 32-bit process:

    cmake -B build -A Win32
    cmake --build build --config Release

## Layout

    src/inject   standalone injector
    src/sniff    payload DLL
