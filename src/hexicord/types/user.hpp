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

#ifndef HEXICORD_TYPES_USER_HPP
#define HEXICORD_TYPES_USER_HPP

#include <string>
#include "hexicord/types/snowflake.hpp"
#include "hexicord/types/image.hpp"
#include "hexicord/json.hpp"
#include "hexicord/types/nlohmann_helper.hpp"

namespace Hexicord {
    struct User {
        /// User objects is not expected to be constructed from user code,
        /// but it's nlohmann/json library's requirement.
        User() = default;

        /// Construct from JSON. For internal use only.
        User(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        /// The user's ID.
        Snowflake id;

        /// The user's username, not unique across the platform (see \ref discriminator).
        std::string username;

        /// The user's 4-digit discord-tag.
        /// (username, discriminator) pair is unique across the platform.
        uint16_t discriminator;

        /// The user's avatar.
        ImageReference<UserAvatar> avatar;

        /// Whether the user belongs to an OAuth2 application.
        bool bot;

        /// Whether the user has two factor enabled on their account.
        bool mfaEnabled;

        /// Whether the email on this account has been verified.
        /// Always false if API user don't have OAuth 'email' scope.
        /// If User object came from gateway Ready event, then this fields
        /// applies to owner's account.
        bool emailVerified;

        /// The user's email. Empty if API user don't have OAuth 'email' scope.
        /// Also always empty for bot accounts.
        std::string email;
    };

} // namespace Hexicord

#endif // HEXICORD_TYPES_USER_HPP
