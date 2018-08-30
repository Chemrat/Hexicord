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
#include <cassert>                         // assert
#include <chrono>                          // std::chrono::steady_clock
#include <unordered_map>                   // std::unordered_map

#include "hexicord/config.hpp"             // HEXICORD_ZLIB HEXICORD_DEBUG_LOG
#include "hexicord/internal/utils.hpp"     // Hexicord::Utils::domainFromUrl

#ifdef HEXICORD_ZLIB
    #include "hexicord/internal/zlib.hpp" // Hexicord::Zlib::decompress
#endif

#ifdef HEXICORD_DEBUG_LOG
    #include <iostream> //
    #define DEBUG_MSG(msg) do { std::cerr <<  "gateway_client.cpp:" << __LINE__ << " " << (msg) << '\n'; } while (false);
#else
    #define DEBUG_MSG(msg)
#endif

// All we need is C++11 compatibile compiler and boost libraries so we can probably run on a lot of platforms.
#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__GNU__)
    #define OS_STR "linux"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    #define OS_STR "bsd"
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #define OS_STR "win32"
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
    #define OS_STR "macos"
#elif defined(__unix__) || defined (__unix) || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE)
    #define OS_STR "unix"
#else
    #define OS_STR "unknown"
#endif

namespace Hexicord {

GatewayClient::GatewayClient(const std::string& token)
    : token_(token) {}

GatewayClient::~GatewayClient() {
    disconnect(2000);
}

void GatewayClient::connect(const std::string& gatewayUrl, int shardId, int shardCount,
                            const nlohmann::json& initialPresence) {
    lastGatewayUrl_ = gatewayUrl + gatewayPathSuffix;
    lastPresence = initialPresence;
    lastSequenceNumber_ = 0;
    sessionId_ = "";
    shardId_ = shardId;
    shardCount_ = shardCount;

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
            recoverConnection();
            break;
        }
        }
    });

    try {
        client.connect(lastGatewayUrl_).wait();
    } catch (std::exception &e) {
        DEBUG_MSG("Failed to connect: " + std::string(e.what()));
    }
}

void GatewayClient::resume(const std::string& gatewayUrl,
                           std::string sessionId, int lastSequenceNumber,
                           int shardId, int shardCount) {

    DEBUG_MSG(std::string("Resuming interrupted gateway session. sessionId=") + sessionId +
              " lastSeq=" + std::to_string(lastSequenceNumber));

    disconnect(2000);

    client.connect(gatewayUrl).wait();
}

void GatewayClient::disconnect(int code) noexcept {
    stop_heartbeat.set_value();

    DEBUG_MSG(std::string("Disconnecting from gateway... code=") + std::to_string(code));
    try {
        if (code != NoCloseEvent && !activeSendMessage) sendMessage(OpCode::EventDispatch, nlohmann::json(code), "CLOSE");
    } catch (...) { // whatever happened - we don't care.
    }

    client.close().wait();
}

void GatewayClient::updatePresence(const nlohmann::json& newPresence) {
    sendMessage(OpCode::StatusUpdate, newPresence);
}

void GatewayClient::sendHello()
{
    nlohmann::json message = {
        { "token" , token_ },
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
        { "presence", lastPresence }
    };

    if (shardId_ != NoSharding && shardCount_ != NoSharding) {
        message.push_back({ "shard", { shardId_, shardCount_ }});
    }

    DEBUG_MSG("Sending Identify message...");
    sendMessage(OpCode::Identify, message);
}

void GatewayClient::sendResume()
{
    DEBUG_MSG("Sending Resume message...");
    sendMessage(OpCode::Resume, {
                    { "token",      token_             },
                    { "session_id", sessionId_          },
                    { "seq",        lastSequenceNumber_ }
                });
}

void GatewayClient::recoverConnection() {
    DEBUG_MSG("Lost gateway connection, recovering...");
    disconnect(NoCloseEvent);

    if (!sessionId_.empty()) {
        DEBUG_MSG("Recoveging session");
        resume(lastGatewayUrl_, sessionId_, lastSequenceNumber_, shardId_, shardCount_);
    } else {
        DEBUG_MSG("Starting new session");
        connect(lastGatewayUrl_, shardId_, shardCount_, lastPresence);
    }
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
        lastSequenceNumber_ = s.get<int>();

        if (eventEnumFromString(t) == Event::Ready) {
            processReady(d);
        }

        eventDispatcher.dispatchEvent(eventEnumFromString(t), d);
        break;
    }
    case OpCode::HeartbeatAck:
        DEBUG_MSG("Gateway heartbeat answered.");
        --unansweredHeartbeats;
        break;
    case OpCode::Heartbeat:
        DEBUG_MSG("Received heartbeat request.");
        sendMessage(OpCode::Heartbeat, nlohmann::json(lastSequenceNumber_));
        ++unansweredHeartbeats;
        break;
    case OpCode::Reconnect:
        DEBUG_MSG("Gateway asked us to reconnect...");
        // It's not recoverConnection duplicate, here Invalid Session error during
        // resume is real error, not just 'start new session instead'.
        disconnect();
        resume(lastGatewayUrl_, sessionId_, lastSequenceNumber_);
        break;
    case OpCode::InvalidSession:
        DEBUG_MSG("Invalid session error.");
        sessionId_ = "";
        lastSequenceNumber_ = 0;
        //throw GatewayError("Invalid session.");
        recoverConnection();
        break;
    case OpCode::Hello:
    {
        heartbeatIntervalMs.store(message.at("d").at("heartbeat_interval"));
        DEBUG_MSG(std::string("Gateway heartbeat interval: ") + std::to_string(heartbeatIntervalMs) + " ms.");

        stop_heartbeat = std::promise<void>();
        std::thread([&](){
            heartbeat(std::move(stop_heartbeat.get_future()));
        }).detach();

        if (!sessionId_.empty()) {
            sendResume();
        } else {
            sendHello();
        }
        break;
    }
    case OpCode::Resume:
    {
        DEBUG_MSG("Got resume");
        break;
    }
    default:
        DEBUG_MSG("Unexpected gateway message.");
        DEBUG_MSG(message);
    }
}

void GatewayClient::sendMessage(GatewayClient::OpCode opCode, const nlohmann::json& payload, const std::string& t) {
    nlohmann::json message = {
        { "op", opCode  },
        { "d",  payload },
    };

    if (!t.empty()) {
        message["t"] = t;
    }

    std::string messageString = message.dump();

    activeSendMessage = true;

    std::cout << "Sending message: " << messageString << std::endl;
    web::websockets::client::websocket_outgoing_message msg;
    msg.set_utf8_message(messageString);

    try {
        client.send(msg).wait();
    } catch (std::exception &e) {
        DEBUG_MSG("client.send failed: " + std::string(e.what()));
        client.close().wait();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        connect(lastGatewayUrl_, shardId_, shardCount_, lastPresence);
    }

    activeSendMessage = false;
}

void GatewayClient::processReady(const nlohmann::json &message)
{
    DEBUG_MSG("Got Ready event. Starting heartbeat and polling...");

    sessionId_          = message.at("session_id");
}

void GatewayClient::heartbeat(std::future<void> &&stop)
{
    bool running = true;
    do {
        auto result = stop.wait_for(std::chrono::milliseconds(heartbeatIntervalMs));
        if (result == std::future_status::timeout) {
            DEBUG_MSG("Sending heartbeat.");
            sendHeartbeat();
        } else {
            running = false;
        }
    } while (running);

    DEBUG_MSG("Heartbeat stopped");
    unansweredHeartbeats.store(0);
}

void GatewayClient::sendHeartbeat() {
    if (unansweredHeartbeats >= 2) {
        DEBUG_MSG("Missing gateway heartbeat answer. Reconnecting...");
        recoverConnection();
        return;
    }

    DEBUG_MSG("Gateway heartbeat sent.");
    sendMessage(OpCode::Heartbeat, nlohmann::json(lastSequenceNumber_));
    ++unansweredHeartbeats;
}

} // namespace Hexicord
