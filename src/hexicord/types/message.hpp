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

#ifndef HEXICORD_TYPES_MESSAGE_HPP
#define HEXICORD_TYPES_MESSAGE_HPP

#include <string>
#include <vector>
#include "hexicord/json.hpp"
#include "hexicord/types/nlohmann_helper.hpp"
#include "hexicord/types/snowflake.hpp"
#include "hexicord/types/embed.hpp"
#include "hexicord/types/user.hpp"
#include "hexicord/types/role.hpp"

namespace Hexicord {
    struct Attachment {
        /// Attachment objects is not meant to be constructed by user code,
        /// but it's nlohmann's library requirement.
        /// \warning Because of this default-constructed Attachment objects
        ///          have **undefined** contents.
        Attachment() = default;

        /// Construct from JSON. For internal use only.
        Attachment(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        /// Attachment id.
        Snowflake id;

        /// Name of file attached.
        std::string filename;

        /// Size of file in bytes
        unsigned size;

        /// Source url of file
        std::string sourceUrl;

        /// A proxied url of file.
        std::string proxifiedUrl;

        /// Dimenisions if image (both 0 otherwise).
        unsigned height = 0, width = 0;
    };

    struct Reaction {
        /// Reaction objects is not meant to be constructed by user code,
        /// but it's nlohmann's library requirement.
        /// \warning Because of this default-constructed Attachment objects
        ///          have **undefined** contents.
        Reaction() = default;

        /// Construct from JSON. For internal use only.
        Reaction(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        /// Times this emoji has been used to react.
        unsigned count;

        /// Whether the current user reacted using this emoji.
        bool me;

        /// Same as \ref Emoji::id.
        Snowflake emojiId;

        /// Same as \ref Emoji::name.
        std::string emojiName;
    };

    struct Message {
        enum Type {
            /// Regular message
            Default = 0,
            /// Recipient added to group DM.
            RecipientAdd = 1,
            /// Recipient removed from group DM.
            RecipientRemove = 2,
            /// Voice call (in DM).
            Call = 3,
            /// Group DM rename.
            ChannelNameChange = 4,
            /// Group DM icon change.
            ChanenlIconChange = 5,
            /// Message pinned/unpinned.
            ChannelPinnedMessage = 6,
            /// Guild member join.
            GuildMemberJoin = 7
        };

        /// Message objects is not meant to be constructed by user code,
        /// but it's nlohmann's library requirement.
        /// \warning Because of this default-constructed Message objects
        ///          have **undefined** contents.
        Message() = default;

        /// Construct from JSON. For internal use only.
        Message(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        /// Id of the message.
        Snowflake id;

        /// Id of the channel the message was sent in.
        Snowflake channelId;

        /// The author of this message.
        /// Id may not refer to a valid user, see \ref webhookId.
        User author;

        /// Text of message (regular, not embed).
        std::string text;

        /// ISO 8601 timestamp, when the message was sent.
        std::string sentTimestamp;

        /// ISO 8601 timestamp, when the message was edited (empty string if never).
        std::string editedTimestamp;

        /// Whether this was a TTS message.
        bool tts;

        /// Whether this message mentions everyone.
        bool mentionEveryone;

        /// Users specifically mentioned in the message.
        std::vector<User> mentionedUsers;

        /// Roles specifically mentioned in this message.
        std::vector<Role> mentionedRoles;

        /// Any attached files.
        std::vector<Attachment> attachments;

        /// Any embedded content.
        std::vector<Embed> embeds;

        /// Reactions to the message.
        std::vector<Reaction> reactions;

        /// Used for optimistic message sending, same as nonce specified when sending message, 0 if none.
        /// Hexicord doesn't provides a way to set nonce when sending messages.
        Snowflake nonce;

        /// Whether this message is pinned.
        bool pinned;

        /// If the message is generated by a webhook, this is the webhook's id, 0 otherwise.
        Snowflake webhookId;

        /// Type of the message, see \ref Type.
        Type type;
    };
} // namespace Hexicord

#endif // HEXICORD_TYPES_MESSAGE_HPP
