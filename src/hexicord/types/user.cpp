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

#include "hexicord/types/user.hpp"

namespace Hexicord {

User::User(const nlohmann::json& json)
    : id             (json.at("id").get<std::string>())
    , username       (json.at("username").get<std::string>())
    , discriminator  (std::stoull(json.at("discriminator").get<std::string>()))
    , avatar         (id, json.at("avatar").get<std::string>())
    , bot            (json.value("bot", false))
    , mfaEnabled     (json.value("mfa_enabled", false))
    , emailVerified  (json.value("verified", false))
    , email          (json.count("email") == 0 || json["email"].is_null() ? "" : json.value("email", "")) {}
    // ^ email field can be null in user object sent in gateway ready event.

nlohmann::json User::dump() const {
    nlohmann::json result = {
        { "id",            std::to_string(id)            },
        { "username",      username                      },
        { "discriminator", std::to_string(discriminator) },
        { "avatar",        avatar.hash                   },
    };

    if (!email.empty()) result["email"]       = email;
    if (emailVerified)  result["verified"]    = emailVerified;
    if (bot)            result["bot"]         = true;
    if (mfaEnabled)     result["mfa_enabled"] = mfaEnabled;

    return result;
}

} // namespace Hexicord
