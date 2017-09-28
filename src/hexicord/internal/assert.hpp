// Hexicord - Discord API library for C++11 using boost libraries.
// Copyright © 2017 Maks Mazurov (fox.cpp) <foxcpp@yandex.ru>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef HEXICORD_INTERNAL_ASSERT_HPP
#define HEXICORD_INTERNAL_ASSERT_HPP

#include <iostream>
#include "hexicord/config.hpp"

#define ASSERT_MSG_WITH_INFO(condition, message, additionalInfo)      \
    do {                                                              \
        if (!(condition))                                             \
            std::cerr << "---- HEXICORD: ASSERTION FAILURE ----\n"    \
                     "  Failed condition: " #condition ".\n"          \
                     "  File: " __FILE__ ".\n"                        \
                     "  Line: " << __LINE__ << ".\n"                  \
                     "\n"                                             \
                     "  " message ".\n"                               \
                     "\n"                                             \
                     "  Additional info: " << additionalInfo << ".\n" \
                     "  Library version: " HEXICORD_VERSION ".\n"     \
                     "\n"                                             \
                     " This message means something went really\n"    \
                     " wrong, library may behave incorrectly or\n"    \
                     " even crash. You must report this as bug!\n";   \
    } while (false)

#define ASSERT_MSG(condition, message) \
    ASSERT_MSG_WITH_INFO(condition, message, "No additional info")

#define ASSERT(condition) \
    ASSERT_MSG(condition, "Unknown error")

#endif // HEXICORD_INTERNAL_ASSERT_HPP

