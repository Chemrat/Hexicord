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

#include "hexicord/types/webhook.hpp"

namespace Hexicord {

Webhook::Webhook(const nlohmann::json& json)
    : id        (json.at    ("id").get<std::string>())
    , guildId   (json.value ("guild_id", "0"))
    , channelId (json.at    ("channel_id").get<std::string>())
    , token     (json.value ("token", "")) {

        if (json.count("user") != 0) {
            creator = Hexicord::User(json["user"]);
        }
    }

nlohmann::json Webhook::dump() const {
    nlohmann::json result = {
        { "id",         std::to_string(id)        },
        { "channel_id", std::to_string(channelId) }
    };

    if (guildId)         result["guild_id"] = std::to_string(guildId);
    if (!token.empty())  result["token"]    = token;
    if (creator)         result["user"]     = creator->dump();

    return result;
}

} // namespace Hexicord
