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

#include "hexicord/types/message.hpp"

namespace Hexicord {
    Attachment::Attachment(const nlohmann::json& json)
        : id            (json.at   ("id")        .get<std::string>())
        , filename      (json.at   ("filename")  .get<std::string>())
        , size          (json.at   ("size")      .get<unsigned>()   )
        , sourceUrl     (json.at   ("url")       .get<std::string>())
        , proxifiedUrl  (json.at   ("proxy_url") .get<std::string>())
        , height        (json.value("height",    0)                 )
        , width         (json.value("width",     0)                 ) {}

    nlohmann::json Attachment::dump() const {
        nlohmann::json result = {
            { "id",         std::to_string(id) },
            { "filename",   filename           },
            { "size",       size               },
            { "url",        sourceUrl          },
            { "proxy_url",  proxifiedUrl       },
        };

        if (height) result["height"] = height;
        if (width)  result["width"]  = width;

        return result;
    }

    Reaction::Reaction(const nlohmann::json& json)
        : count     (json.at("count")            .get<unsigned>()   )
        , me        (json.at("me")               .get<bool>()       )
        , emojiId   (json.at("emoji").at("id")   .get<std::string>())
        , emojiName (json.at("emoji").at("name") .get<std::string>()) {}

    nlohmann::json Reaction::dump() const {
        return {
            { "count", count },
            { "me",    me    },
            { "emoji", {
                { "id",   std::to_string(emojiId) },
                { "name", emojiName               }
            }}
        };
    }

    Message::Message(const nlohmann::json& json)
        : id              (json.at("id")               .get<std::string>()            )
        , channelId       (json.at("channel_id")       .get<std::string>()            )
        , author          (json.at("author")           .get<User>()                   )
        , text            (json.at("content")          .get<std::string>()            )
        , sentTimestamp   (json.at("timestamp")        .get<std::string>()            )
        , editedTimestamp (!json["edited_timestamp"].is_null() ?
                           json.at("edited_timestamp") .get<std::string>() : "")
        , tts             (json.at("tts")              .get<bool>()                   )
        , mentionEveryone (json.at("mention_everyone") .get<bool>()                   )
        , mentionedUsers  (json.at("mentions")         .get<std::vector<User>>()      )
        , mentionedRoles  (json.at("mention_roles")    .get<std::vector<Role>>()      )
        , attachments     (json.at("attachments")      .get<std::vector<Attachment>>())
        , embeds          (json.at("embeds")           .get<std::vector<Embed>>()     )
        , reactions       (json.value("reactions", std::vector<Reaction>())           )
        , nonce           (json.count("nonce") != 0 && !json["nonce"].is_null() ?
                            json["nonce"].get<std::string>() : "0")
        , pinned          (json.at("pinned").get<bool>())
        , webhookId       (json.value("webhook_id", "0"))
        , type            (Type(json.at("type").get<int>())) {}

    nlohmann::json Message::dump() const {
        nlohmann::json result = {
            { "id"                 , std::to_string(id)        },
            { "channel_id"         , std::to_string(channelId) },
            { "author"             , author                    },
            { "content"            , text                      },
            { "timestamp"          , sentTimestamp             },
            { "tts"                , tts                       },
            { "mention_everyone"   , mentionEveryone           },
            { "mentions"           , mentionedUsers            },
            { "mention_roles "     , mentionedRoles            },
            { "attachments"        , attachments               },
            { "embeds"             , embeds                    },
            { "pinned"             , pinned                    },
            { "type"               , int(type)                 }
        };

        result["edited_timestamp"] = editedTimestamp.empty() ? nlohmann::json(nullptr) : nlohmann::json(editedTimestamp);
        result["nonce"]            = nonce == 0              ? nlohmann::json(nullptr) : nlohmann::json(nonce);
        if (webhookId != 0)     result["webhook_id"] = std::to_string(webhookId);
        if (!reactions.empty()) result["reactions"]  = reactions;

        return result;
    }
} // namespace Hexicord
