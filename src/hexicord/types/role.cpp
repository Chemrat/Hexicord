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

#include "hexicord/types/role.hpp"

namespace Hexicord {
    Role::Role(const nlohmann::json& json)
        : id            (json.at("id")          .get<std::string>())
        , name          (json.at("name")        .get<std::string>())
        , color         (json.at("color")       .get<unsigned>())
        , permissions   (json.at("permissions") .get<uint64_t>())
        , managed       (json.at("managed")     .get<bool>())
        , mentionable   (json.at("mentionable") .get<bool>()) {}

    nlohmann::json Role::dump() const {
        return {
            { "id",           id                     },
            { "color",        color                  },
            { "hoist",        hoist                  },
            { "position",     position               },
            { "permissions",  uint64_t(permissions)  },
            { "managed",      managed                },
            { "mentionable",  mentionable            }
        };
    }

    PermissionsOverwrite::PermissionsOverwrite(const nlohmann::json& json)
        : forId  (json.at("id").get<std::string>())
        , type   (json.at("type") == "role" ? Role : Member)
        , allow  (json.at("allow").get<uint64_t>())
        , deny   (json.at("deny") .get<uint64_t>()) {}

    nlohmann::json PermissionsOverwrite::dump() const {
        return {
            { "id",     std::to_string(forId)               },
            { "type",   type == Role ? "role" : "member"    },
            { "allow",  uint64_t(allow)                     },
            { "deny",   uint64_t(deny)                      },
        };
    }
} // namespace Hexicord

