#ifndef HEXICORD_TYPES_USER_HPP
#define HEXICORD_TYPES_USER_HPP

#include <string>
#include "hexicord/types/snowflake.hpp"
#include "hexicord/types/image.hpp"
#include "hexicord/json.hpp"
#include "hexicord/types/nlohmann_helper.hpp"

namespace Hexicord {
    struct User {
        /// User objects is not expected to be constructed from user code.
        User() = delete;

        /// Construct from JSON. For internal use only.
        User(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        /// The user's ID.
        Snowflake id;

        /// The user's username, not unique across the platform (see \ref discriminator).
        std::string username;

        /// The user's 4-digit discord-tag.
        /// (username, discriminator) pair is unique across the platform.
        uint16_t discriminator;

        /// The user's avatar.
        ImageReference<UserAvatar> avatar;

        /// Whether the user belongs to an OAuth2 application.
        bool bot;

        /// Whether the user has two factor enabled on their account.
        bool mfaEnabled;

        /// Whether the email on this account has been verified.
        /// Always false if API user don't have OAuth 'email' scope.
        bool emailVerified;

        /// The user's email. Empty if API user don't have OAuth 'email' scope.
        std::string email;
    };

} // namespace Hexicord

#endif // HEXICORD_TYPES_USER_HPP
