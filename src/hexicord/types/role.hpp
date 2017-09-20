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

