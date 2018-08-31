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

#include "hexicord/gateway_client.hpp"

#include <thread>
#include <chrono>                          // std::chrono::steady_clock

#include "hexicord/config.hpp"             // HEXICORD_ZLIB HEXICORD_DEBUG_LOG
#include "internal/utils.hpp"     // Hexicord::Utils::domainFromUrl
#include "internal/os_string.h"

#ifdef HEXICORD_ZLIB
    #include "hexicord/internal/zlib.hpp" // Hexicord::Zlib::decompress
#endif

#ifdef HEXICORD_DEBUG_LOG
    #include <iostream> //
    #define DEBUG_MSG(msg) do { std::cerr <<  "gateway_client.cpp:" << __LINE__ << " " << (msg) << '\n'; } while (false);
#else
    #define DEBUG_MSG(msg)
#endif

/**
 * Hypothetical threads (cpprestsdk actually uses a lot more?)
 *  A User thread (calls connect and moves on)
 *  B Callback thread (calls message handlers)
 *  C Heartbeat thread
 *
 * Possible disconnect scenarios:
 *  A Sending message results in an error
 *  A GateWay client is destroyed
 *  B We recieve reconnect request from discord
 *  B We recieve CLOSE or corrupted message
 *  C Heartbeat failure
 *
 * Possible states
 *  Not connected at all
 *  Connected normally
 *  Connection failed and we're in recovery state. We should drop wss connection but keep session ID
 *  Resume failed: we should drop wss connection again(?) and start clean
 */

namespace Hexicord {

GatewayClient::GatewayClient(const std::string& token)
    : _token(token)
{
    client.set_message_handler([&](web::websockets::client::websocket_incoming_message msg){
        switch (msg.message_type()) {
        case web::websockets::client::websocket_message_type::text_message:
        {
            const auto body = msg.extract_string().get();
            std::cout << "Message: " << body << std::endl;
            const nlohmann::json message = nlohmann::json::parse(body);
            processMessage(message);
            break;
        }
        case web::websockets::client::websocket_message_type::binary_message:
        {
            concurrency::streams::container_buffer<std::vector<uint8_t>> ret_data;
            msg.body().read_to_end(ret_data).wait();
            std::cout << "Compressed message" << std::endl;
            const nlohmann::json message = nlohmann::json::parse(Zlib::decompress(ret_data.collection()));
            processMessage(message);
            break;
        }
        case web::websockets::client::websocket_message_type::close:
        {
            DEBUG_MSG("Websocket closed");
            disconnect();
            connect();
            break;
        }
        }
    });
}

GatewayClient::~GatewayClient() {
    disconnect(2000);
}

void GatewayClient::init(const std::string& gatewayUrl,
                            int shardId,
                            int shardCount,
                            const nlohmann::json& initialPresence)
{
    _gatewayUrl = gatewayUrl + gatewayPathSuffix;
    _shardId = shardId;
    _shardCount = shardCount;
    _presence = initialPresence;
}

void GatewayClient::invalidateSession()
{
    _lastSequenceNumber = 0;
    _sessionId = "";
}

void GatewayClient::connect()
{
    if (_gatewayUrl.empty()) {
        DEBUG_MSG("Not initalized");
        return;
    }

    while (_state.load() == State::Disconnected) {
        try {
            client.connect(_gatewayUrl).wait();
            _state.store(State::Connected);
        } catch (std::exception &e) {
            DEBUG_MSG("Failed to connect: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    }
}

void GatewayClient::resume(const std::string& gatewayUrl,
                           std::string sessionId, int lastSequenceNumber,
                           int shardId, int shardCount) {

    DEBUG_MSG(std::string("Resuming interrupted gateway session. sessionId=") + sessionId +
              " lastSeq=" + std::to_string(lastSequenceNumber));

    disconnect(2000);

    try {
        client.connect(gatewayUrl).wait();
        _state.store(State::Connected);
    } catch (std::exception &e) {
        DEBUG_MSG("Failed to connect: " + std::string(e.what()));
    }
}

void GatewayClient::disconnect(int code) noexcept {
    _state.store(State::Disconnected);
    stop_heartbeat.set_value();

    DEBUG_MSG(std::string("Disconnecting from gateway... code=") + std::to_string(code));

    if (code != NoCloseEvent) {
        sendMessage(OpCode::EventDispatch, nlohmann::json(code), "CLOSE");
    }

    client.close().wait();
}

void GatewayClient::updatePresence(const nlohmann::json& newPresence) {
    sendMessage(OpCode::StatusUpdate, newPresence);
}

void GatewayClient::sendIdentify()
{
    nlohmann::json message = {
        { "token" , _token },
        { "properties", {
            { "$os", OS_STR },
            { "$browser", "hexicord" },
            { "$device", "hexicord" }
        }},
#ifdef HEXICORD_ZLIB
        { "compress", true },
#else
        { "compress", false },
#endif
        { "large_threshold", 250 }, // should be changeble
        { "presence", _presence }
    };

    if (_shardId != NoSharding && _shardCount != NoSharding) {
        message.push_back({ "shard", { _shardId, _shardCount }});
    }

    DEBUG_MSG("Sending Identify message...");
    sendMessage(OpCode::Identify, message);
}

void GatewayClient::sendResume()
{
    DEBUG_MSG("Sending Resume message...");
    sendMessage(OpCode::Resume, {
                    { "token",      _token             },
                    { "session_id", _sessionId          },
                    { "seq",        _lastSequenceNumber }
                });
}

void GatewayClient::recoverConnection() {
    DEBUG_MSG("Lost gateway connection, recovering...");
    disconnect(NoCloseEvent);

    connect();
}

void GatewayClient::processMessage(const nlohmann::json& message) {
    std::cout << "Processing message OP: " << message.at("op").get<int>() << std::endl;
    switch (message.at("op").get<int>()) {
    case OpCode::EventDispatch:
    {
        const auto &t = message.at("t");
        const auto &s = message.at("s");
        const auto &d = message.at("d");
        DEBUG_MSG(std::string("Gateway Event: t=") + t.get<std::string>() +
                  " s=" + std::to_string(s.get<int>()));
        _lastSequenceNumber = s.get<int>();

        if (eventEnumFromString(t) == Event::Ready) {
            processReady(d);
        } else if (eventEnumFromString(t) == Event::Resumed) {
            processResume(d);
        }

        eventDispatcher.dispatchEvent(eventEnumFromString(t), d);
        break;
    }
    case OpCode::Heartbeat:
    {
        DEBUG_MSG("Received heartbeat request.");
        sendMessage(OpCode::Heartbeat, nlohmann::json(_lastSequenceNumber));
        ++unansweredHeartbeats;
        break;
    }
    case OpCode::Reconnect:
    {
        DEBUG_MSG("Gateway asked us to reconnect...");
        // It's not recoverConnection duplicate, here Invalid Session error during
        // resume is real error, not just 'start new session instead'.
        disconnect();
        resume(_gatewayUrl, _sessionId, _lastSequenceNumber);
        break;
    }
    case OpCode::InvalidSession:
    {
        _state.store(State::Connected);
        DEBUG_MSG("Invalid session error.");
        const auto resumable = message.at("d").get<bool>();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (resumable == false) {
            _sessionId = "";
            _lastSequenceNumber = 0;
            sendIdentify();
        } else {
            sendResume();
        }

        break;
    }
    case OpCode::Hello:
    {
        heartbeatIntervalMs.store(message.at("d").at("heartbeat_interval"));
        DEBUG_MSG(std::string("Gateway heartbeat interval: ") + std::to_string(heartbeatIntervalMs) + " ms.");

        stop_heartbeat = std::promise<void>();
        std::thread([&](){
            heartbeat(std::move(stop_heartbeat.get_future()));
        }).detach();

        if (!_sessionId.empty()) {
            sendResume();
        } else {
            sendIdentify();
        }
        break;
    }
    case OpCode::HeartbeatAck:
    {
        DEBUG_MSG("Gateway heartbeat answered.");
        --unansweredHeartbeats;
        break;
    }
    default:
        DEBUG_MSG("Unexpected OpCode recieved");
        DEBUG_MSG(message);
    }
}

bool GatewayClient::sendMessage(GatewayClient::OpCode opCode, const nlohmann::json& payload, const std::string& t) {
    if (_state.load() != State::Ready) {
        DEBUG_MSG("SendMessage called with non-ready State");
        return false;
    }

    nlohmann::json message = {
        { "op", opCode  },
        { "d",  payload },
    };

    if (!t.empty()) {
        message["t"] = t;
    }

    std::string messageString = message.dump();

    std::cout << "Sending message: " << messageString << std::endl;
    web::websockets::client::websocket_outgoing_message msg;
    msg.set_utf8_message(messageString);

    try {
        client.send(msg).wait();
        return true;
    } catch (std::exception &e) {
        DEBUG_MSG("client.send failed: " + std::string(e.what()));
        return false;
    }
}

void GatewayClient::processReady(const nlohmann::json &message)
{
    DEBUG_MSG("Got Ready event. Starting heartbeat and polling...");
    _sessionId          = message.at("session_id");
    _state.store(State::Ready);
}

void GatewayClient::processResume(const nlohmann::json &message)
{
    DEBUG_MSG("Got Resume event");
    _state.store(State::Ready);
}

void GatewayClient::heartbeat(std::future<void> &&stop)
{
    bool running = true;
    do {
        auto result = stop.wait_for(std::chrono::milliseconds(heartbeatIntervalMs));
        if (result == std::future_status::timeout) {
            DEBUG_MSG("Sending heartbeat.");
            running = sendHeartbeat();
        } else {
            running = false;
        }
    } while (running);

    DEBUG_MSG("Heartbeat stopped");
    unansweredHeartbeats.store(0);
}

bool GatewayClient::sendHeartbeat() {
    if (unansweredHeartbeats >= 2) {
        DEBUG_MSG("Missing gateway heartbeat answer. Reconnecting...");
        disconnect();
        connect();
        return false;
    }

    DEBUG_MSG("Gateway heartbeat sent.");
    sendMessage(OpCode::Heartbeat, nlohmann::json(_lastSequenceNumber));
    ++unansweredHeartbeats;
    return true;
}

} // namespace Hexicord
