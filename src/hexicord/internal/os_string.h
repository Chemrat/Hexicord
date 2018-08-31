#ifndef OS_STRING_H
#define OS_STRING_H

// All we need is C++11 compatibile compiler and boost libraries so we can probably run on a lot of platforms.
#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__GNU__)
    #define OS_STR "linux"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    #define OS_STR "bsd"
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #define OS_STR "win32"
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
    #define OS_STR "macos"
#elif defined(__unix__) || defined (__unix) || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE)
    #define OS_STR "unix"
#else
    #define OS_STR "unknown"
#endif

#endif // OS_STRING_H
