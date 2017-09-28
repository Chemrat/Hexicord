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

#ifndef HEXICORD_TYPES_ROLE_HPP
#define HEXICORD_TYPES_ROLE_HPP

#include <string>
#include "hexicord/json.hpp"
#include "hexicord/permission.hpp"
#include "hexicord/types/snowflake.hpp"
#include "hexicord/types/nlohmann_helper.hpp"

namespace Hexicord {
    struct Role {
        Role() = default;

        /// Construct from JSON. For internal use only.
        Role(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        /// Role id.
        Snowflake id;

        /// Role name.
        std::string name = "new role";

        /// Integer representation of hexadecimal color code. 0 means no color.
        unsigned color = 0;

        /// Whatever this role is pinned in the user listing
        bool hoist = false;

        /// Position of this role.
        int position = 99999999;

        /// Permissions.
        Permissions permissions;

        /// Whether this role is managed by an integration
        bool managed = false;

        /// Whether this role is mentionable.
        bool mentionable = false;
    };

    struct PermissionsOverwrite {
        PermissionsOverwrite() = default;

        /// Construct from JSON. For internal use only.
        PermissionsOverwrite(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        /// Role or user ID.
        Snowflake forId;

        /// Whatever override for role or member..
        enum Type { Role, Member } type;

        /// Permissions set to allow.
        Permissions allow;

        /// Permissions set to deny.
        Permissions deny;
    };
} // namespace Hexicord

#endif // HEXICORD_TYPES_ROLE_HPP

