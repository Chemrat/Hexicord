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

#include "hexicord/types/emoji.hpp"

namespace Hexicord {

Emoji::Emoji(const nlohmann::json& json)
    : id            (json.at("id")             .get<uint64_t>()              )
    , name          (json.at("name")           .get<std::string>()           )
    , roles         (json.at("roles")          .get<std::vector<Snowflake>>())
    , requireColons (json.at("require_colons") .get<bool>()                  )
    , managed       (json.at("managed")        .get<bool>()                  ) {}

nlohmann::json Emoji::dump() const {
    return {
        { "id",             id            },
        { "name",           name          },
        { "roles",          roles         },
        { "require_colons", requireColons },
        { "managed",        managed       }
    };
}

} // namespace Hexicord

