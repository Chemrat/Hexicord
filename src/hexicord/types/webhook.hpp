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

#ifndef HEXICORD_TYPES_WEBHOOK_HPP
#define HEXICORD_TYPES_WEBHOOK_HPP

#include <string>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include "hexicord/json.hpp"
#include "hexicord/types/nlohmann_helper.hpp"
#include "hexicord/types/snowflake.hpp"
#include "hexicord/types/user.hpp"

namespace Hexicord {
    struct Webhook {
        /// Webhook objects is not meant to be constructed by user code,
        /// but it's nlohmann/json library's requirement.
        Webhook() = default;

        /// Construct from JSON. For internal use only.
        Webhook(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        /// The id of the webhook.
        Snowflake id;

        /// The guild id this webhook is for.
        /// Sometimes can be 0 for unknown reasons. Ask on Discord API server.
        Snowflake guildId;

        /// The channel id this webhook is for.
        Snowflake channelId;

        /// The user this webhook was created by (not present when getting a webhook with its token).
        boost::optional<User> creator;

        /// The secure token of the webhook, empty if unknown.
        std::string token;
    };
} // namespace Hexicord

#endif // HEXICORD_TYPES_WEBHOOK_HPP
