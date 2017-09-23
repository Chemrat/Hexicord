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
