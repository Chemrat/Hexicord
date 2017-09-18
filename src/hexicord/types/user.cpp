#include "hexicord/types/user.hpp"

namespace Hexicord {

User::User(const nlohmann::json& json)
    : id             (std::stoull(json.at("id").get<std::string>())
    , username       (json.at("username").get<std::string>())
    , discriminator  (std::stoull(json.at("discriminator").get<std::string>()))
    , avatar         (id, json.at("avatar").get<std::string>())
    , bot            (json.at("bot").get<bool>())
    , mfaEnabled     (json.at("mfa_enabled").get<bool>())
    , emailVerified  (json.value("verified", false).get<bool>())
    , email          (json.value("email", "").get<std::string>()) {}

nlohmann::json User::dump() const {
    if (!email.empty()) {
        // If we have email, return object with "verified" and "email" fields.
        return {
            { "id",            std::to_string(id)            },
            { "username",      username                      },
            { "discriminator", std::to_string(discriminator) },
            { "avatar",        avatar.hash                   },
            { "bot",           bot                           },
            { "mfa_enabled",   mfaEnabled                    },
            { "verified",      emailVerified                 },
            { "email",         email                         }
        };
    } else {
        return {
            { "id",            std::to_string(id)            },
            { "username",      username                      },
            { "discriminator", std::to_string(discriminator) },
            { "avatar",        avatar.hash                   },
            { "bot",           bot                           },
            { "mfa_enabled",   mfaEnabled                    }
        };
    }
}

} // namespace Hexicord
