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

