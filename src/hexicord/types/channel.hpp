#ifndef HEXICORD_TYPES_CHANNEL_HPP
#define HEXICORD_TYPES_CHANNEL_HPP

#include <boost/variant/variant.hpp>
#include "hexicord/json.hpp"
#include "hexicord/types/nlohmann_helper.hpp"
#include "hexicord/types/snowflake.hpp"
#include "hexicord/types/user.hpp"
#include "hexicord/types/role.hpp"

namespace Hexicord {
    struct BaseGuildChannel {
        /// Channel ID.
        Snowflake id;

        /// Guild ID. Can be 0 if not present in JSON
        /// it's constructed from (for example channels from GuildCreate event).
        Snowflake guildId;

        /// The ID of parent category, if any, 0 otherwise.
        Snowflake categoryId;

        /// The name of the channel (2-100 characters).
        std::string name;

        /// Sorting position of the channel.
        unsigned position;

        /// Explicit permission overwrites for members and roles.
        std::vector<PermissionsOverwrite> permissionOverwrites;
    protected:
        BaseGuildChannel() = default;
        BaseGuildChannel(const nlohmann::json& json);

        nlohmann::json dump() const;
    };

    struct TextChannel : BaseGuildChannel {
        TextChannel() = default;
        TextChannel(const nlohmann::json& json);

        nlohmann::json dump() const;

        /// The channel topic (0-1024 characters).
        std::string topic = "";

        /// The id of the last message sent in this channel (may not point to an existing or valid message).
        Snowflake lastMessageId;
    };

    struct VoiceChannel : BaseGuildChannel {
        VoiceChannel() = default;

        /// Construct from JSON. For internal use only.
        VoiceChannel(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;

        /// The bitrate (in bits) of the voice channel.
        unsigned bitrate    = 64000;

        /// The users limit of the voice channel.
        unsigned usersLimit = 0;
    };


    struct Category : BaseGuildChannel {
        // TODO: Should we make Category default-constructible?
        Category() = default;

        /// Construct from JSON. For internal use only.
        Category(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;


        bool nsfw;
    };

    struct DmChannel {
        // TODO: Should we make DmChannel default-constructible?
        DmChannel() = delete;

        /// Construct from JSON. For internal use only.
        DmChannel(const nlohmann::json& json);

        // Serialize to JSON. For internal use only.
        nlohmann::json dump() const;

        /// DM ID.
        Snowflake id;

        /// The id of the last message sent in this channel (may not point to an existing or valid message).
        Snowflake lastMessageId;

        /// The recipient of the DM.
        User recipient;
    };

    struct GroupDmChannel {
        // TODO: Should we make GroupDmChannel default-constructible?
        GroupDmChannel() = delete;

        /// Construct from JSON. For internal use only.
        GroupDmChannel(const nlohmann::json& json);

        /// Serialize to JSON. For internal use only.
        nlohmann::json dump() const;

        // Group DM ID.
        Snowflake id;

        /// The id of the last message sent in this channel (may not point to an existing or valid message).
        Snowflake lastMessageId;

        /// Application id of the group DM creator if it is bot-created, 0 otherwise.
        Snowflake applicationId;

        /// The recipients of the DM.
        std::vector<User> recipients;
    };

    enum class ChannelType {
        Text            = 0,
        Voice           = 2,
        Category        = 4,
        Dm              = 1,
        GroupDm         = 3
    };

    ChannelType channelType(const nlohmann::json& json);

    template<typename T>
    constexpr ChannelType channelType();

    template<> constexpr ChannelType channelType<TextChannel>()    { return ChannelType::Text;     }
    template<> constexpr ChannelType channelType<VoiceChannel>()   { return ChannelType::Voice;    }
    template<> constexpr ChannelType channelType<Category>()       { return ChannelType::Category; }
    template<> constexpr ChannelType channelType<DmChannel>()      { return ChannelType::Dm;       }
    template<> constexpr ChannelType channelType<GroupDmChannel>() { return ChannelType::GroupDm;  }

    using Channel = boost::variant<TextChannel, VoiceChannel, Category, DmChannel, GroupDmChannel>;

    void from_json(const nlohmann::json& json, Channel& channel);
    void to_json(nlohmann::json& json, const Channel& channel);
} // namespace Hexicord

#endif // HEXICORD_TYPES_CHANNEL_HPP
