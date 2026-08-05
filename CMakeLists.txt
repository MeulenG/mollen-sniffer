cmake_minimum_required(VERSION 3.16)
project(mollen-sniffer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "mollen-sniffer is Windows-only")
endif()

# The 3.3.5a client is a 32-bit process: a 64-bit injector can't inject into
# it, and a 64-bit DLL can't load inside it. Fail at configure time rather
# than with a confusing runtime error.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "32-bit build required - configure with: cmake -B build -A Win32")
endif()

# Standalone injector.
add_executable(mollen-inject src/inject/main.cpp)

# Payload DLL, loaded into the client by the injector.
add_library(mollen-sniff SHARED src/sniff/dllmain.cpp)
