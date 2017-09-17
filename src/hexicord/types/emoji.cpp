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

