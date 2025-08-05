#include <tgbot/net/CurlHttpClient.h>
#include <tgbot/tgbot.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>


#ifndef HAVE_CURL
#error "HAVE_CURL is not defined"
#endif



using namespace std;
using namespace TgBot;

int main() {
    string token(getenv("TOKEN") ? getenv("TOKEN") : "");
    if (token.empty()) {
        fprintf(stderr, "❌ TOKEN environment variable is not set.\n");
        return 1;
    }

    CurlHttpClient curlHttpClient;

    Bot bot(token, curlHttpClient);
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });

    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
