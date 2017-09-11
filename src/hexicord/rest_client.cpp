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

#include <hexicord/rest_client.hpp>
#include <thread>                                     // std::this_thread::sleep_for
#include <chrono>                                     // std::chrono::seconds, std::chrono::milliseconds
#include <fstream>                                    // std::ifstream
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/beast/http/error.hpp>                 // boost::beast::http::error::end_of_stream
#include <hexicord/exceptions.hpp>
#include <hexicord/internal/utils.hpp>                // Utils::getRatelimitDomain, Utils::domainFromUrl

#if defined(HEXICORD_DEBUG_LOG)
    #include <iostream>
    #define DEBUG_MSG(msg) do { std::cerr <<  "rest_client.cpp:" << __LINE__ << " " << (msg) << '\n'; } while (false)
#else
    #define DEBUG_MSG(msg)
#endif

namespace Hexicord {
    RestClient::RestClient(boost::asio::io_service& ioService, const std::string& token) 
        : restConnection(new REST::HTTPSConnection(ioService, "discordapp.com"))
        , token(token)
        , ioService(ioService) {

        // It's strange but Discord API requires "DiscordBot" user-agent for any connections
        // including non-bots. Referring to https://discordapp.com/developers/docs/reference#user-agent
        restConnection->connectionHeaders.insert({ "User-Agent", "DiscordBot (" HEXICORD_GITHUB ", " HEXICORD_VERSION ")" });
    }

    std::string RestClient::getGatewayUrl() {
        restConnection->connectionHeaders.insert({ "Authorization", std::string("Bearer ") + token });

        nlohmann::json response = sendRestRequest("GET", "/gateway");
        return response["url"];
    }

    std::pair<std::string, int> RestClient::getGatewayUrlBot() {
        restConnection->connectionHeaders.insert({ "Authorization", std::string("Bot ") + token });

        nlohmann::json response = sendRestRequest("GET", "/gateway/bot");
        return { response["url"].get<std::string>(), response["shards"].get<unsigned>() };
    }

    nlohmann::json RestClient::sendRestRequest(const std::string& method, const std::string& endpoint,
                                           const nlohmann::json& payload,
                                           const std::unordered_map<std::string, std::string>& query,
                                           const std::vector<REST::MultipartEntity>& multipart) {

        if (!restConnection->isOpen()) restConnection->open();

        REST::HTTPRequest request;

        request.method  = method;
        request.path    = restBasePath + endpoint + Utils::makeQueryString(query);
        request.version = 11;

        prepareRequestBody(request, payload, multipart);

        request.headers.insert({ "Accept", "application/json" });

#ifdef HEXICORD_RATELIMIT_PREDICTION 
        // Make sure we can do request without getting ratelimited.
        ratelimitLock.down(Utils::getRatelimitDomain(endpoint));
#endif

        REST::HTTPResponse response;
        try {
            DEBUG_MSG(std::string("Sending REST request: ") + method + " " + request.path + " " + payload.dump());
            response = restConnection->request(request);
        } catch (boost::system::system_error& excp) {
            if (excp.code() != boost::beast::http::error::end_of_stream &&
                excp.code() != boost::asio::error::broken_pipe &&
                excp.code() != boost::asio::error::connection_reset) throw;

            DEBUG_MSG("HTTP Connection closed by remote. Reopenning and retrying.");
            {
                REST::HeadersMap prevHeaders = std::move(restConnection->connectionHeaders);
                restConnection.reset(new REST::HTTPSConnection(ioService, "discordapp.com"));
                restConnection->connectionHeaders = std::move(prevHeaders);
                restConnection->open();
            }
            
            return sendRestRequest(method, endpoint, payload, query, multipart);
        }

        if (response.body.empty()) {
            return {};
        }

        nlohmann::json jsonResp = nlohmann::json::parse(response.body);

#ifdef HEXICORD_RATELIMIT_PREDICTION
        updateRatelimitsIfPresent(endpoint, response.headers);
#endif

        if (response.statusCode / 100 != 2) {
            if (response.statusCode == 429) {
#ifdef HEXICORD_RATELIMIT_HIT_AS_ERROR
                throw RatelimitHit(Utils::getRatelimitDomain(endpoint));
#else 
                std::this_thread::sleep_for(std::chrono::seconds(jsonResp["retry_after"].get<unsigned>()));
                return sendRestRequest(method, endpoint, payload, query);
#endif 
            }

            DEBUG_MSG("Got non-2xx HTTP status code.");
            DEBUG_MSG(jsonResp.dump(4));
            throwRestError(response, jsonResp);
        }

        return jsonResp;
    }

    nlohmann::json RestClient::getChannel(Snowflake channelId) {
        return sendRestRequest("GET", std::string("/channels/") + std::to_string(channelId));
    }

    nlohmann::json RestClient::modifyChannel(Snowflake channelId,
                                         boost::optional<std::string> name,
                                         boost::optional<int> position,
                                         boost::optional<std::string> topic,
                                         boost::optional<unsigned> bitrate,
                                         boost::optional<unsigned short> usersLimit) {
        nlohmann::json payload;
        if (name) {
            if (name->size() > 100 || name->size() < 2) {
                throw InvalidParameter("name", "name size out of range (should be 2-100).");
            }
            payload.emplace("name", *name);
        }
        if (position) {
            payload.emplace("position", *position);
        }
        if (topic && (bitrate || usersLimit)) {
            throw InvalidParameter("bitrate", "Passing both voice-only and text-only channel arguments.");
        }
        if (topic) {
            if (topic->size() > 1024) {
                throw InvalidParameter("topic", "topic size out of range (should be 0-1024).");
            }
            payload.emplace("topic", *topic);
        }
        if (bitrate) {
            if (*bitrate < 8000 || *bitrate > 128000) {
                throw InvalidParameter("bitrate", "bitrate out of range (should be 8000-128000).");
            }
            payload.emplace("bitrate", *bitrate);
        }
        if (usersLimit) {
            if (*usersLimit > 99) {
                throw InvalidParameter("userslimit", "usersLimit out of range (should be 0-99).");
            }
            payload.emplace("users_limit", *usersLimit);
        }
        if (payload.empty()) {
            throw InvalidParameter("", "No arguments passed to modifyChannel.");
        }

        return sendRestRequest("POST", std::string("/channels/") + std::to_string(channelId), payload);
    }

    nlohmann::json RestClient::deleteChannel(Snowflake channelId) {
        return sendRestRequest("DELETE", std::string("/channels/") + std::to_string(channelId));
    }

    nlohmann::json RestClient::getMessages(Snowflake channelId, RestClient::After afterId, unsigned limit) {
        if (limit > 100 || limit == 0) {
            throw InvalidParameter("limit", "limit out of range (should be 1-100).");
        }

        return sendRestRequest("GET", std::string("/channels/") + std::to_string(channelId) + "/messages",
                               {}, {{ "after", std::to_string(afterId.id) },
                                    { "limit", std::to_string(limit) }});
    }

    nlohmann::json RestClient::getMessages(Snowflake channelId, RestClient::Before afterId, unsigned limit) {
        if (limit > 100 || limit == 0) {
            throw InvalidParameter("limit", "limit out of range (should be 1-100).");
        }

        return sendRestRequest("GET", std::string("/channels/") + std::to_string(channelId) + "/messages",
                               {}, {{ "before", std::to_string(afterId.id) },
                                    { "limit", std::to_string(limit) }});
    }

    nlohmann::json RestClient::getMessages(Snowflake channelId, RestClient::Around afterId, unsigned limit) {
        if (limit > 100 || limit == 1) {
            throw InvalidParameter("limit", "limit out of range (should be 2-100).");
        }

        return sendRestRequest("GET", std::string("/channels/") + std::to_string(channelId) + "/messages",
                               {}, {{ "around", std::to_string(afterId.id) },
                                    { "limit", std::to_string(limit) }});
    }

    nlohmann::json RestClient::getMessage(Snowflake channelId, Snowflake messageId) {
        return sendRestRequest("GET", std::string("/channels/") + std::to_string(channelId) + 
                               "/messsages/" + std::to_string(messageId));
    }

    nlohmann::json RestClient::getPinnedMessages(Snowflake channelId) {
        return sendRestRequest("GET", std::string("/channels/") + std::to_string(channelId) +
                                                  "/pins");
    }

    void RestClient::pinMessage(Snowflake channelId, Snowflake messageId) {
        sendRestRequest("PUT", std::string("/channels/") + std::to_string(channelId) +
                                           "/pins/"      + std::to_string(messageId));
    }

    void RestClient::unpinMessage(Snowflake channelId, Snowflake messageId) {
        sendRestRequest("DELETE", std::string("/channels/") + std::to_string(channelId) +
                                              "/pins/"      + std::to_string(messageId));
    }

    void RestClient::editChannelRolePermissions(Snowflake channelId, Snowflake roleId,
                                    Permissions allow, Permissions deny) {

        sendRestRequest("PUT", std::string("/channels/")   + std::to_string(channelId) +
                                           "/permissions/" + std::to_string(roleId),
            {
                { "allow", int(allow) },
                { "deny",  int(deny)  },
                { "type",  "role"     }
            });
    }

    void RestClient::editChannelUserPermissions(Snowflake channelId, Snowflake userId,
                                    Permissions allow, Permissions deny) {

        sendRestRequest("PUT", std::string("/channels/")   + std::to_string(channelId) +
                                           "/permissions/" + std::to_string(userId),
            {
                { "allow", int(allow) },
                { "deny",  int(deny)  },
                { "type",  "member"   }
            });
    }

    void RestClient::deleteChannelPermissions(Snowflake channelId, Snowflake overrideId) {
        sendRestRequest("DELETE", std::string("/channels/")   + std::to_string(channelId) +
                                              "/permissions/" + std::to_string(overrideId));
    }

    void RestClient::kickFromGroupDm(Snowflake groupDmId, Snowflake userId) {
        sendRestRequest("DELETE", std::string("/channels/")  + std::to_string(groupDmId) +
                                              "/recipients/" + std::to_string(userId));
    }

    void RestClient::addToGroupDm(Snowflake groupDmId, Snowflake userId,
                              const std::string& accessToken, const std::string& nick) {

        sendRestRequest("PUT", std::string("/channels/")  + std::to_string(groupDmId) +
                                           "/recipients/" + std::to_string(userId),
                        {
                            { "access_token", accessToken },
                            { "nick",         nick        }
                        });
    }

    void RestClient::triggerTypingIndicator(Snowflake channelId) {
        sendRestRequest("POST", std::string("/channels/") + std::to_string(channelId) + "/typing");
    }

    nlohmann::json RestClient::sendTextMessage(Snowflake channelId, const std::string& text, bool tts) {
        if (text.size() > 2000) throw InvalidParameter("text", "text out of range (should be 0-2000).");

        return sendRestRequest("POST", std::string("/channels/") + std::to_string(channelId) + "/messages",
                {
                  { "contents", text },
                  { "tts",      tts  }
                });
    }

    nlohmann::json RestClient::sendFile(Snowflake channelId, const File& file) {
        return sendRestRequest("POST", std::string("/channels/") + std::to_string(channelId) + "/messages",
                               {}, {}, { fileToMultipartEntity(file) });
    }

    nlohmann::json RestClient::editMessage(Snowflake channelId, Snowflake messageId, const std::string& text) {
        if (text.size() > 2000) {
            throw InvalidParameter("text", "text size out of range (should be 0-1024)");
        }
        return sendRestRequest("PATCH", std::string("/channels/") + std::to_string(channelId) +
                               "/messages/" + std::to_string(messageId),
                               {{ "content", text }});
    }

    void RestClient::deleteMessage(Snowflake channelId, Snowflake messageId) {
        sendRestRequest("DELETE", std::string("/channels/") + std::to_string(channelId) + "/messages/" +
                        std::to_string(messageId));
    }

    void RestClient::deleteMessages(Snowflake channelId, const std::vector<Snowflake>& messageIds) {
        sendRestRequest("DELETE", std::string("/channels/") + std::to_string(channelId) + "/messages/bulk-delete",
                        {{ "messages", messageIds }});
    }

    void RestClient::addReaction(Snowflake channelId, Snowflake messageId, Snowflake emojiId) {
        sendRestRequest("PUT", std::string("/channels/") + std::to_string(channelId) +
                                           "/messages/"  + std::to_string(messageId) +
                                           "/reactions/" + std::to_string(emojiId)   +
                                           "/@me");
    }

    void RestClient::removeReaction(Snowflake channelId, Snowflake messageId, Snowflake emojiId, Snowflake userId) {
        sendRestRequest("DELETE", std::string("/channels/") + std::to_string(channelId) +
                                              "/messages/"  + std::to_string(messageId) +
                                              "/reactions/" + std::to_string(emojiId)   +
                                             (userId ? std::to_string(userId) : std::string("/@me")));
    }

    nlohmann::json RestClient::getReactions(Snowflake channelId, Snowflake messageId, Snowflake emojiId) {
        return sendRestRequest("GET", std::string("/channels/") + std::to_string(channelId) +
                                                  "/messages/"  + std::to_string(messageId) +
                                                  "/reactions/" + std::to_string(emojiId));
    }

    void RestClient::resetReactions(Snowflake channelId, Snowflake messageId) {
        sendRestRequest("DELETE", std::string("/channels/") + std::to_string(channelId) +
                                              "/messages/"  + std::to_string(messageId) +
                                              "/reactions");
    }

    nlohmann::json RestClient::getGuild(Snowflake id) {
        return sendRestRequest("GET", std::string("/guilds/") + std::to_string(id));
    }

    nlohmann::json RestClient::createGuild(const nlohmann::json& newGuildObject) {
        return sendRestRequest("POST", "/guilds", newGuildObject);
    }

    nlohmann::json RestClient::modifyGuild(Snowflake id, const nlohmann::json& changedFields) {
        return sendRestRequest("PATCH", std::string("/guilds/") + std::to_string(id), changedFields);
    }

    nlohmann::json RestClient::getBans(Snowflake guildId) {
        return sendRestRequest("GET", std::string("/guilds/") + std::to_string(guildId) +
                                                  "/bans");
    }

    void RestClient::banMember(Snowflake guildId, Snowflake userId, unsigned deleteMessagesDays) {
        sendRestRequest("PUT", std::string("/guilds/") + std::to_string(guildId) +
                                           "/bans"     + std::to_string(userId),
                        {}, {{ "delete-message-days", std::to_string(deleteMessagesDays) }});
    }

    void RestClient::unbanMember(Snowflake guildId, Snowflake userId) {
        sendRestRequest("DELETE", std::string("/guilds/") + std::to_string(guildId) +
                                              "/bans"     + std::to_string(userId));
    }

    nlohmann::json RestClient::getMe() {
        return sendRestRequest("GET", std::string("/users/@me"));
    }

    nlohmann::json RestClient::getUser(Snowflake id) {
        return sendRestRequest("GET", std::string("/users/") + std::to_string(id));
    }

    nlohmann::json RestClient::setUsername(const std::string& newUsername) {
        if (newUsername.size() < 2 || newUsername.size() > 32) {
            throw InvalidParameter("newUsername", "newUseranme size out of range (should be 2-32)");
        }
        if (newUsername == "discordtag" || newUsername == "everyone" || newUsername == "here") {
            throw InvalidParameter("newUsername", "newUsername should not be 'discordtag', 'everyone' or 'here'");
        }
        unsigned short foundGraves = 0;
        for (char ch : newUsername) {
            if (ch == '@' || ch == '#' || ch == ':') {
                throw InvalidParameter("newUsername", "newUsername contains foribbden characters ('@', '#' or ':')");
            }
            if (ch == '`') {
                ++foundGraves;
                if (foundGraves == 3) {
                    throw InvalidParameter("newUsername", "newUsername contains forbidden substring: '```'");
                }
            } else {
                foundGraves = 0;
            }
        }

        return sendRestRequest("PATCH", "/users/@me", {{ "username", newUsername }});
    }

    nlohmann::json RestClient::setAvatar(const std::vector<uint8_t>& avatarBytes, AvatarFormat format) {
        std::string mimeType;

        if (format == Gif  || (format == Detect && Utils::Magic::isGif(avatarBytes)))  mimeType = "image/gif";
        if (format == Jpeg || (format == Detect && Utils::Magic::isJfif(avatarBytes))) mimeType = "image/jpeg";
        if (format == Png  || (format == Detect && Utils::Magic::isPng(avatarBytes)))  mimeType = "image/png";

        if (mimeType.empty()) {
            throw InvalidParameter("avatarBytes", "Failed to detect avatar format.");
        }

        std::string dataUrl = std::string("data:") + mimeType + ";base64," + Utils::base64Encode(avatarBytes);
        return sendRestRequest("PATCH", "/users/@me", {{ "avatar", dataUrl }});
    }

    nlohmann::json RestClient::setAvatar(std::istream&& avatarStream, AvatarFormat format) {
        // TODO: Implement stream sending.
        std::vector<uint8_t> avatarBytes{std::istreambuf_iterator<char>(avatarStream), std::istreambuf_iterator<char>()};
        return setAvatar(avatarBytes, format);
    }

    nlohmann::json RestClient::getUserGuilds(unsigned short limit, Snowflake startId, bool before) {
        std::unordered_map<std::string, std::string> query;
        if (limit != 100) {
            query.insert({ "limit", std::to_string(limit) });
        }
        if (startId != 0) {
            query.insert({ before ? "before" : "after", std::to_string(startId) });
        }
        return sendRestRequest("GET", "/users/@me/guilds", {}, query);
    }

    void RestClient::leaveGuild(Snowflake guildId) {
        sendRestRequest("DELETE", std::string("/users/@me/guilds/") + std::to_string(guildId));
    }

    nlohmann::json RestClient::getUserDms() {
        return sendRestRequest("GET", "/users/@me/channels");
    }

    nlohmann::json RestClient::createDm(Snowflake recipientId) {
        return sendRestRequest("POST", "/users/@me/channels", {{ "recipient_id", recipientId }});
    }


    nlohmann::json RestClient::createGroupDm(const std::vector<Snowflake>& accessTokens,
                                         const std::unordered_map<Snowflake, std::string>& nicks) {

        return sendRestRequest("POST", "/users/@me/channels", {{ "access_tokens", accessTokens },
                                                               { "nicks",         nicks        }});
    }

    nlohmann::json RestClient::getConnections() {
        return sendRestRequest("GET", "/users/@me/connections");
    }

    nlohmann::json RestClient::getInvites(Snowflake guildId) {
        return sendRestRequest("GET", std::string("/guilds/") + std::to_string(guildId) +
                                                  "/invites");
    }

    nlohmann::json RestClient::getInvite(const std::string& inviteCode) {
        return sendRestRequest("GET", std::string("/invites/") + inviteCode);
    }

    nlohmann::json RestClient::revokeInvite(const std::string& inviteCode) {
        return sendRestRequest("DELETE", std::string("/invites/") + inviteCode);
    }

    nlohmann::json RestClient::acceptInvite(const std::string& inviteCode) {
        return sendRestRequest("POST", std::string("/invites/") + inviteCode);
    }

    nlohmann::json RestClient::getChannelInvites(Snowflake channelId) {
        return sendRestRequest("GET", std::string("/channels/") + std::to_string(channelId) +
                                                  "/invites");
    }

    nlohmann::json RestClient::createInvite(Snowflake channelId, unsigned maxAgeSecs,
                                        unsigned maxUses,
                                        bool temporaryMembership,
                                        bool unique) {
        nlohmann::json payload;
        if (maxAgeSecs != 86400)  payload["max_age"]              = maxAgeSecs;
        if (temporaryMembership)  payload["temporary_membership"] = true;
        if (unique)               payload["unique"]               = true;

        return sendRestRequest("DELETE", std::string("/channels") + std::to_string(channelId), payload);
    }

    void RestClient::prepareRequestBody(REST::HTTPRequest& request,
                                        const nlohmann::json& payload,
                                        const std::vector<REST::MultipartEntity>& elements) {


        if (elements.empty()) {
            if (payload.is_null() || payload.empty()) return;

            request.headers["Content-Type"] = "application/json";

            std::string jsonStr = payload.dump();
            std::vector<uint8_t> payloadBytes(jsonStr.begin(), jsonStr.end());

            request.body = payloadBytes;
        } else {
            std::vector<REST::MultipartEntity> actualMultipartElements;
            actualMultipartElements.reserve(elements.size() + 1);

            // We add JSON payload as first element.
            if (!payload.is_null() && !payload.empty()) {
                std::string bodyStr = Utils::urlEncode(payload.dump());
                actualMultipartElements.push_back({
                        /* name:              */ "payload_json",
                        /* filename:          */ "",
                        /* additionalHeaders: */ {{ "Content-Type", "application/json" }},
                        /* body               */ std::vector<uint8_t>(bodyStr.begin(), bodyStr.end())
                        });
            }

            for (const auto& element : elements) actualMultipartElements.push_back(element);

            REST::HTTPRequest tempRequest = REST::buildMultipartRequest(actualMultipartElements);
            request.headers["Content-Type"] = tempRequest.headers["Content-Type"];
            request.body                    = tempRequest.body;
        }
    }

    void RestClient::throwRestError(const REST::HTTPResponse& response,
                                const nlohmann::json& payload) {

            int code = -1;
            if (payload.find("code") != payload.end()) code = payload["code"];
            if (payload.find("message") != payload.end()) {
                if (code / 10000 == 1) {
                    throw UnknownEntity(payload["message"], code);
                }
                if (code / 10000 == 5) {
                    throw LimitReached(payload["message"], code); 
                }

                throw RESTError(payload["message"].get<std::string>(), code, response.statusCode);
            }
            

            /* REST API sometimes return errors about invalid paramters in
             * undocumented format, like this:
             * ```json
             * {
             *  "parameter_name": [ "whats_wrong" ]
             * }
             * ```
             *
             * We iterate through received payload and check if it looks like JSON
             * above, if so - we throw InvalidParameter.
             */
            for (auto param : payload.get<std::unordered_map<std::string, nlohmann::json> >()) { 
                if (param.second.is_array() && param.second.size() == 1 &&
                    param.second[0].is_string()) {
                    // check for array with one string inside ^

                    // param.first => parameter name
                    // param.second[0] => error description
                    throw InvalidParameter(param.first, param.second[0]);
                }
            }

            throw RESTError("Unknown error");
    }

#ifdef HEXICORD_RATELIMIT_PREDICTION 
    void RestClient::updateRatelimitsIfPresent(const std::string& endpoint, const REST::HeadersMap& headers) {
        auto remainingIt = headers.find("X-RateLimit-Remaining");
        auto limitIt     = headers.find("X-RateLimit-Limit");
        auto resetIt     = headers.find("X-RateLimit-Reset");

        unsigned remaining  =  remainingIt  !=  headers.end() ? std::stoi(remainingIt->second)  : 0;
        unsigned limit      =  limitIt      !=  headers.end() ? std::stoi(limitIt->second)      : 0;
        time_t reset        =  resetIt      !=  headers.end() ? std::stoll(resetIt->second)     : 0;

        if (remaining && limit && reset) {
            ratelimitLock.refreshInfo(Utils::getRatelimitDomain(endpoint), remaining, limit, reset);
        }
    }
#endif // HEXICORD_RATELIMIT_PREDICTION

    REST::MultipartEntity RestClient::fileToMultipartEntity(const File& file) {
        return {
                /* name:              */ file.filename,
                /* filename:          */ file.filename,
                /* additionalHeaders: */ {},
                /* body:              */ file.bytes
               };
    }
} // namespace Hexicord
