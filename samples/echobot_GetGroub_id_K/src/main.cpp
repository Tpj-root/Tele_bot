#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

string chatTypeToString(Chat::Type type) {
    switch (type) {
        case Chat::Type::Private: return "private";
        case Chat::Type::Group: return "group";
        case Chat::Type::Supergroup: return "supergroup";
        case Chat::Type::Channel: return "channel";
        default: return "unknown";
    }
}

int main() {
    string token(getenv("TOKEN"));
    printf("Token: %s\n", token.c_str());

    Bot bot(token);

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        int64_t chatId = message->chat->id;
        string chatType = chatTypeToString(message->chat->type);
        string sender = message->from->username;

        printf("Chat ID: %ld\n", chatId);
        printf("Chat Type: %s\n", chatType.c_str());
        printf("From: @%s\n", sender.c_str());
        printf("Message: %s\n", message->text.c_str());

        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }

        string reply = "Chat ID: " + to_string(chatId) +
                       "\nChat Type: " + chatType +
                       "\nFrom: @" + sender +
                       "\nMessage: " + message->text;

        bot.getApi().sendMessage(chatId, reply);
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
