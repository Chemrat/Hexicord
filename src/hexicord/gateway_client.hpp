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

#ifndef HEXICORD_GATEWAY_CLIENT_HPP
#define HEXICORD_GATEWAY_CLIENT_HPP

#define _TURN_OFF_PLATFORM_STRING
#include <cpprest/ws_client.h>

#include <future>
#include <thread>
#include <cstdint>                       // uint8_t
#include <memory>                        // std::unique_ptr
#include <stdexcept>                     // std::runtime_error
#include <string>                        // std::string
#include <vector>                        // std::vector
#include <hexicord/event_dispatcher.hpp> // Hexicord::Event, Hexicord::EventDispatcher
#include <hexicord/json.hpp>             // nlohmann::json
namespace Hexicord { class TLSWebSocket; }
namespace boost { namespace asio { class io_service; } }

namespace Hexicord {
    /**
     *  Thrown if gateway API error occurs (disconnect, OP 9 Invalid Session, unexcepted message, etc).
     */
    struct GatewayError : public std::runtime_error {
        GatewayError(const std::string& message, int disconnectCode = -1)
            : std::runtime_error(message)
            , disconnectCode(disconnectCode) {}

        /**
         *  Contains gateway disconnect code if error caused
         *  by disconnection, -1 otherwise.
         */
        const int disconnectCode;
    };


    class GatewayClient {
    public:
        static constexpr int NoSharding = -1;
        static constexpr int NoCloseEvent = -1;

        GatewayClient(const std::string& token);
        ~GatewayClient();

        GatewayClient(const GatewayClient&) = delete;
        GatewayClient(GatewayClient&&) = default;

        GatewayClient& operator=(const GatewayClient&) = delete;
        GatewayClient& operator=(GatewayClient&&) = default;

        void init(const std::string& gatewayUrl,
                  int shardId,
                  int shardCount,
                  const nlohmann::json& initialPresence);
        void invalidateSession();

        enum class State {
            Ready,
            Disconnected,
            Connected,
        };

        std::atomic<State> _state{State::Disconnected};

        /**
         * Connect and identify to gateway.
         *
         * Either this method of \ref resume should be called and succeed
         * in order to start event receiving. You should pass gateway URL
         * received by calling to \ref RestClient::getGatewayUrl or
         * \ref RestClient::getGatewayUrlBot.
         *
         * Also if your client is a bot and you're using sharding, then you
         * need to pass shardId and shardCount, otherwise you should leave both
         * parameters to NoSharding.
         *
         * initialPresense specifies status to set instantly after connection,
         * defaults to "online" without game playing.
         *
         * This method will throw GatewayError if there is something
         * wrong with identify data (for example, revoked or invalid token).
         *
         * This method will throw ConnectionError if failed to open connection.
         * But it's unlikely to happen because most ASIO operations have really
         * long timeout.
         *
         * \sa \ref GatewayClient::resume
         *     \ref RestClient::getGatewayUrl
         *     \ref RestClient::getGatewayUrlBot
         *
         * \internal
         * **Implementation**
         *
         * If connection is not open - open it, send identify payload, wait for first
         * gateway message, if it's ready event - start gateway polling, if it's
         * invalid session error - throw exception.
         */
        void connect();

        /**
         * Resume interrupted gateway session.
         *
         * Can be invoked instead of \ref connect if you're recovering from
         * bot crash and want to  receive lost events.
         *
         * If connection already open and client is identified - behavior
         * is undefined.
         *
         * Also if your client is a bot and you're used sharding in previous
         * session then you should pass used shardId and shardCount to this
         * method.
         *
         * May throw InvalidSession exception, in this case you can't resume
         * session and have to use \ref connect.
         *
         * \sa \ref GatewayClient::connect
         *     \ref RestClient::getGatewayUrl
         *     \ref RestClient::getGatewayUrlBot
         *
         * \internal
         * **Implementation**
         *
         * If connection is not open - open it, send resume event, block until
         * either Resumed event or Invalid Session received.
         * If Resumed event - start gateway polling and heartbeating.
         * If Invalid session - throw exception.
         */
        void resume(const std::string& gatewayUrl,
                    std::string sessionId, int lastSequenceNumber,
                    int shardId = NoSharding, int shardCount = NoSharding);

        /**
         * Disconnect from gateway with sending Close event
         * with specified code.
         *
         * You can also specify NoCloseEvent (-1) code to disable Close event
         * sending, but you should note that Close event updates your bot
         * status to offline, so disconnecting without this event would
         * result in bot still "online" for a around minute.
         *
         * \note Close event may not be sent and error will be ignored because
         *       this function is marked as noexcept.
         *
         * \internal
         * **Implementation**
         *
         * Send close event, shutdown WebSocket, free socket.
         */
        void disconnect(int code = 2000) noexcept;

        /**
         * Block until specified event type is received, then return payload of it.
         *
         * Still runs other event handlers and send heartbeats, so it may throw
         * ConnectionError, GatewayError or anything you throw from handlers.
         *
         * It's better to use async handlers, since behavior of this method is
         * not well defined in all cases.
         */
        //nlohmann::json waitForEvent(Event type);

        /**
         * Update presence (user status).
         */
        void updatePresence(const nlohmann::json& newPresence);

        void sendIdentify();
        void sendResume();

        /**
         * Event dispatcher instance used for gateway
         * event dispatching.
         *
         * Safe to reassign to default-constructed value if you want to clear
         * handlers (however, no handlers or event polling should be running
         * in other threads, if you have any).
         *
         * \internal
         * **Implementation**
         *
         * \ref EventDispatcher::dispatchEvent called by processMessage if
         * payload contains message.
         */
        EventDispatcher eventDispatcher;

        inline const std::string& token() const {
            return _token;
        }

        inline const std::string& sessionId() const {
            return _sessionId;
        }

        inline int lastSequenceNumber() const {
            return _lastSequenceNumber;
        }

        inline const std::string& lastGatewayUrl() const {
            return _gatewayUrl;
        }
private:
        enum OpCode {
            EventDispatch        = 0,
            Heartbeat            = 1,
            Identify             = 2,
            StatusUpdate         = 3,
            VoiceStateUpdate     = 4,
            VoiceServerPing      = 5,
            Resume               = 6,
            Reconnect            = 7,
            RequestGuildMembers  = 8,
            InvalidSession       = 9,
            Hello                = 10,
            HeartbeatAck         = 11,
        };

        // Disconnect without Close event, try to resume session, if failed - start new session,
        // if failed - throw InvalidSession.
        void recoverConnection();

        // Poll gateway connection using async read while poll = true, calls
        // processMessage for each message if skipMessages is not set.
        // Saves last received message in lastMessage.

        nlohmann::json lastMessage;

        void processMessage(const nlohmann::json& message);
        bool sendMessage(OpCode opCode, const nlohmann::json& payload = {}, const std::string& t = "");

        void processReady(const nlohmann::json& message);
        void processResume(const nlohmann::json& message);

        // Set if sendMessage entered.
        bool activeSendMessage = false;

        std::promise<void> stop_heartbeat;
        void heartbeat(std::future<void> &&stop);

        // Heartbeat information, used by asyncHeartbeat and sendHeartbeat.
        //bool heartbeat = true;
        std::atomic<int> heartbeatIntervalMs = 0;
        std::atomic<int> unansweredHeartbeats = 0;

        // Send heartbeat, if we don't have answer for two heartbeats - reconnect and return.
        bool sendHeartbeat();

        // Session information.
        std::string _sessionId;
        std::string _gatewayUrl;
        std::string _token;
        int _shardId = NoSharding;
        int _shardCount = NoSharding;
        int _lastSequenceNumber = 0;
        nlohmann::json _presence;

        constexpr static char gatewayPathSuffix[] = "/?v=6&encoding=json";

        web::websockets::client::websocket_callback_client client;
    };
}

#endif // HEXICORD_GATEWAY_CLIENT_HPP
