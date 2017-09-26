#include <cstdlib>
#include <iostream>
#include <hexicord/gateway_client.hpp>
#include <hexicord/rest_client.hpp>

int main(int argc, char** argv) {
    const char* botToken = std::getenv("BOT_TOKEN");
    if (!botToken) {
        std::cerr << "Set bot token using BOT_TOKEN enviroment variable.\n"
                  << "E.g. env BOT_TOKEN=token_here " << argv[0] << '\n';
        return 1;
    }

    const char* ownerIdStr = std::getenv("OWNER_ID");
    if (!ownerIdStr) {
        std::cerr << "OWNER_ID is not set, echo-bot shutdown can't be used.\n";
    }
    Hexicord::Snowflake ownerId = ownerIdStr ? Hexicord::Snowflake(ownerIdStr) : Hexicord::Snowflake();

    boost::asio::io_service ioService;
    Hexicord::GatewayClient gclient(ioService, botToken);
    Hexicord::RestClient    rclient(ioService, botToken);

    /// Stores off/on state for each channel.
    std::unordered_map<Hexicord::Snowflake, bool> switchFlags;

    /// me user object for id comparsion.
    nlohmann::json me;

    gclient.eventDispatcher.addHandler(Hexicord::Event::Ready, [&me](const nlohmann::json& json) {
        me = json["user"];
    });

    gclient.eventDispatcher.addHandler(Hexicord::Event::MessageCreate, [&](const nlohmann::json& json) {
        Hexicord::Snowflake messageId(json["id"].get<std::string>());
        Hexicord::Snowflake channelId(json["channel_id"].get<std::string>());
        Hexicord::Snowflake senderId(json["author"]["id"].get<std::string>());

        // Avoid responing to messages of bot.
        if (senderId == Hexicord::Snowflake(me["id"].get<std::string>())) return;

        std::string text = json["content"];

        // Construct response message.
        std::string messageInfo =
            std::string("Message ID: `") + std::to_string(messageId) +
                      "`\nChannel ID: `"  + std::to_string(channelId) +
                      "`\nSender ID: `"   + std::to_string(senderId)  + "`\n" +
                      "\n" + text + "\n\n";

        // Log response message to stdout.
        std::cout << messageInfo;

        auto it = switchFlags.find(channelId);
        if (it == switchFlags.end()) {
            switchFlags.emplace(channelId, false);

            // Emplace doesn't returns iterator to inserted item, we have to use find again.
            it = switchFlags.find(channelId);
        }
        bool& switchFlag = it->second;

        // We don't have command handling for bots in Hexicord yet, just comparing strings now.

        if (text == "echo-bot turn-on") {
            if (switchFlag) {
                rclient.sendTextMessage(channelId, "Already turned on.");
                return;
            }

            std::cerr << "Turning on for channel " << channelId << '\n';
            switchFlag = true; // This will update switchFlag in std::unordered_map, because it's a reference.
            rclient.sendTextMessage(channelId, "Turned on. Use `echo-bot turn-off` to turn off.");
            return;
        }

        if (text == "echo-bot turn-off") {
            if (!switchFlag) {
                rclient.sendTextMessage(channelId, "Already turned off.");
                return;
            }
            std::cerr << "Turning off for channel " << channelId << '\n';
            switchFlag = false; // This will update switchFlag in std::unordered_map, because it's a reference.
            rclient.sendTextMessage(channelId, "Turned off. Use `echo-bot turn-on` to turn on.");
            return;
        }

        if (text == "echo-bot shutdown") {
            if (senderId == ownerId) {
                rclient.sendTextMessage(channelId, "Goodbye!");
                gclient.disconnect();
                std::exit(1); // std::exit doesn't calls destuctors of local variables, thus explicit disconnect is required.
            } else {
                rclient.sendTextMessage(channelId, "Only my owner can use this command.");
            }
        }

        // Finally, if we have echo turned on for this channel and message is not a commmand...
        if (switchFlag) {
            rclient.sendTextMessage(channelId, messageInfo);
        }
    });

    // Connect to gateway (getGatewayUrlBot returns pair, where first is gateway URL).
    // We also set status to "Playing echo-bot turn-on".
    gclient.connect(rclient.getGatewayUrlBot().first,
                    Hexicord::GatewayClient::NoSharding, Hexicord::GatewayClient::NoSharding,
                    {
                        { "since", nullptr   },
                        { "status", "online" },
                        { "game", {{ "name", "echo-bot turn-on"}, { "type", 0 }}},
                        { "afk", false }
                    });


    // Run for undetermined amount of time.
    ioService.run();
}
