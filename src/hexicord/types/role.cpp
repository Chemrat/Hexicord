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

