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

#ifndef HEXICORD_TYPES_EMOJI_HPP
#define HEXICORD_TYPES_EMOJI_HPP

#include <string>
#include <vector>
#include "hexicord/json.hpp"
#include "hexicord/types/snowflake.hpp"
#include "hexicord/types/nlohmann_helper.hpp"

namespace Hexicord {
    struct Emoji {
        /// Emoji objects is not expected to be created by user code.
        Emoji() = delete;

        /// Construct from JSON. For internal use only.
        Emoji(const nlohmann::json& json);

        /// Serialize to JSON.
        nlohmann::json dump() const;

        /// Emoji ID.
        Snowflake id;

        /// Emoji name.
        std::string name;

        /// Roles this emoji is whitelisted to.
        std::vector<Snowflake> roles;

        /// Whether this emoji must be wrapped in colons.
        bool requireColons;

        /// Whether this emoji is managed.
        bool managed;
    };
} // namespace Hexicord

#endif // HEXICORD_TYPES_EMOJI_HPP
