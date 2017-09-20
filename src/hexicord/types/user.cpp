#include "hexicord/types/user.hpp"

namespace Hexicord {

User::User(const nlohmann::json& json)
    : id             (json.at("id").get<std::string>())
    , username       (json.at("username").get<std::string>())
    , discriminator  (std::stoull(json.at("discriminator").get<std::string>()))
    , avatar         (id, json.at("avatar").get<std::string>())
    , bot            (json.value("bot", false))
    , mfaEnabled     (json.value("mfa_enabled", false))
    , emailVerified  (json.value("verified", false))
    , email          (json.count("email") == 0 || json["email"].is_null() ? "" : json.value("email", "")) {}
    // ^ email field can be null in user object sent in gateway ready event.

nlohmann::json User::dump() const {
    nlohmann::json result = {
        { "id",            std::to_string(id)            },
        { "username",      username                      },
        { "discriminator", std::to_string(discriminator) },
        { "avatar",        avatar.hash                   },
    };

    if (!email.empty()) result["email"]       = email;
    if (emailVerified)  result["verified"]    = emailVerified;
    if (bot)            result["bot"]         = true;
    if (mfaEnabled)     result["mfa_enabled"] = mfaEnabled;

    return result;
}

} // namespace Hexicord
