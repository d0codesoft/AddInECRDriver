// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define LIB_EXPORTS

#define U_STATIC_IMPLEMENTATION

// add headers that you want to pre-compile here
#include "framework.h"

// Standard libraries
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Detect OS
#if defined(_WIN32) || defined(_WIN64)
#define OS_WINDOWS
#include <windows.h>
#include <tchar.h>
#elif defined(__APPLE__) || defined(__MACH__)
#define OS_MACOS
#include <unistd.h>
#elif defined(__linux__)
#define OS_LINUX
#include <unistd.h>
#include <pthread.h>
#else
#error "Unsupported operating system"
#endif

#endif //PCH_H
