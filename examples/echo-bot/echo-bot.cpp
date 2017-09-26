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

    boost::asio::io_service ioService;
    Hexicord::GatewayClient gclient(ioService, botToken);
    Hexicord::RestClient    rclient(ioService, botToken);

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

        // Construct response message.
        std::string messageInfo =
            std::string("Message ID: `") + std::to_string(messageId) +
                     "`\nChannel ID: `"  + std::to_string(channelId) +
                     "`\nSender ID:  `"  + std::to_string(senderId)  + "`\n" +
                     "\n" + json["content"].get<std::string>() + "\n\n";

        // Log response message to stdout.
        std::cout << messageInfo;

        rclient.sendTextMessage(channelId, messageInfo);
    });

    // Connect to gateway (getGatewayUrlBot returns pair, where first is gateway URL).
    gclient.connect(rclient.getGatewayUrlBot().first);

    // Run for undetermined amount of time.
    ioService.run();
}
