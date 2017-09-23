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
