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
