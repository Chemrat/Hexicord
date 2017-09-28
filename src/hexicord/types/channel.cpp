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

#include "hexicord/types/channel.hpp"
#include <boost/variant/get.hpp>

namespace Hexicord {

BaseGuildChannel::BaseGuildChannel(const nlohmann::json& json)
    : id                   (json.at    ("id")       .get<std::string>())
    , guildId              (json.count("guild_id") != 0 ? json.value("guild_id", "0") : "0")
    , categoryId           (json.count("parent_id") != 0 && !json["parent_id"].is_null() ? json.value ("parent_id", "0") : "0")
    , name                 (json.at    ("name")     .get<std::string>())
    , position             (json.at    ("position") .get<unsigned>()   )
    , permissionOverwrites (json.at    ("permission_overwrites").get<std::vector<PermissionsOverwrite>>()) {}

nlohmann::json BaseGuildChannel::dump() const {
    nlohmann::json result = {
        { "id",                    std::to_string(id)       },
        { "name",                  name                     },
        { "position",              position                 },
        { "permission_overwrites", permissionOverwrites     }
    };

    if (categoryId != 0) result["parent_id"] = std::to_string(categoryId);
    if (guildId    != 0) result["guild_id"]  = std::to_string(guildId);
    return result;
}

TextChannel::TextChannel(const nlohmann::json& json)
    : BaseGuildChannel(json)
    , topic         (json["topic"] != nullptr ? json.at("topic").get<std::string>() : "")
    , lastMessageId (json["last_message_id"].is_null() ? "0" : json.at("last_message_id").get<std::string>())
    , nsfw          (json.value("nsfw", false)) {}

nlohmann::json TextChannel::dump() const {
    nlohmann::json json = BaseGuildChannel::dump();

    json["topic"]           = topic.empty() ? nlohmann::json(nullptr) : nlohmann::json(topic);
    json["last_message_id"] = lastMessageId != 0 ? nlohmann::json(std::to_string(lastMessageId)) : nlohmann::json(nullptr);

    if (nsfw) json["nsfw"] = true;

    return json;
}

VoiceChannel::VoiceChannel(const nlohmann::json& json)
    : BaseGuildChannel(json)
    , bitrate       (json.at("bitrate")    .get<unsigned>())
    , usersLimit    (json.at("user_limit").get<unsigned>()) {}


nlohmann::json VoiceChannel::dump() const {
    nlohmann::json json = BaseGuildChannel::dump();

    json["bitrate"]     = bitrate;
    json["user_limit"] = usersLimit;

    return json;
}

Category::Category(const nlohmann::json& json)
    : BaseGuildChannel(json)
    , nsfw (json.value("nsfw", false)) {}

nlohmann::json Category::dump() const {
    nlohmann::json result = BaseGuildChannel::dump();

    result["nsfw"] = nsfw;

    return result;
}

DmChannel::DmChannel(const nlohmann::json& json)
    : id             (json.at("id")              .get<std::string>())
    , lastMessageId  (json["last_message_id"].is_null() ? "0" : json.at("last_message_id").get<std::string>())
    , recipient      (json.at("recipients")[0]   .get<User>()       ) {}

nlohmann::json DmChannel::dump() const {
    nlohmann::json json = {
        { "id", uint64_t(id)                },
        { "last_message_id", lastMessageId  },
        { "recipients", { recipient.dump() }}
    };

    json["last_message_id"] = lastMessageId != 0 ? nlohmann::json(std::to_string(lastMessageId)) : nlohmann::json(nullptr);

    return json;
}

GroupDmChannel::GroupDmChannel(const nlohmann::json& json)
    : id             (json.at("id")              .get<std::string>()      )
    , lastMessageId  (json["last_message_id"].is_null() ? "0" : json.at("last_message_id").get<std::string>())
    , recipients     (json.at("recipients")      .get<std::vector<User>>()) {}

nlohmann::json GroupDmChannel::dump() const {
    nlohmann::json json = {
        { "id", uint64_t(id)               },
        { "recipients", recipients         }
    };

    json["last_message_id"] = lastMessageId != 0 ? nlohmann::json(std::to_string(lastMessageId)) : nlohmann::json(nullptr);

    return json;
}

ChannelType channelType(const nlohmann::json& json) {
    return static_cast<ChannelType>(json.at("type").get<unsigned>());
}

void from_json(const nlohmann::json& json, Channel& channel) {
    ChannelType type = channelType(json);
    switch (type) {
    case ChannelType::Text:
        channel = Channel(TextChannel(json));
        break;
    case ChannelType::Voice:
        channel = Channel(VoiceChannel(json));
        break;
    case ChannelType::Category:
        channel = Channel(Category(json));
        break;
    case ChannelType::Dm:
        channel = Channel(DmChannel(json));
        break;
    case ChannelType::GroupDm:
        channel = Channel(GroupDmChannel(json));
        break;
    }
}

void to_json(nlohmann::json& json, const Channel& channel) {
    if (channel.type() == typeid(TextChannel)) {
        json = boost::get<TextChannel>(channel);
    } else if (channel.type() == typeid(VoiceChannel)) {
        json = boost::get<VoiceChannel>(channel);
    } else if (channel.type() == typeid(Category)) {
        json = boost::get<Category>(channel);
    } else if (channel.type() == typeid(DmChannel)) {
        json = boost::get<DmChannel>(channel);
    } else if (channel.type() == typeid(GroupDmChannel)) {
        json = boost::get<GroupDmChannel>(channel);
    } else {
        assert(false);
    }
}

} // namespace Hexicord
