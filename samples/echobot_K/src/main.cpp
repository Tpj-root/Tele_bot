#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;


void printNumber(const Bot& bot, Chat::Ptr chat) {
    for (int i = 1; i <= 10; ++i) {
        bot.getApi().sendMessage(chat->id, to_string(i));
    }
}


int main() {
    string token(getenv("TOKEN")); // Get bot token from environment
    printf("Token: %s\n", token.c_str());

    Bot bot(token); // Create bot instance

    // Handle /start command
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });

    // Handle any other text message
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return; // Skip if it's /start
        }
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });

//  terminal prints 1 to 10
// bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
//     printNumber(); // This will print 1 to 10 in terminal
//     bot.getApi().sendMessage(message->chat->id, "Hi!");
// });


    
    bot.getEvents().onCommand("end", [&bot](Message::Ptr message) {
        printNumber(bot, message->chat);
        //bot.getApi().sendMessage(message->chat->id, "Hi!");
    });




    // Handle Ctrl+C to exit
    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook(); // Ensure long polling is used

        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start(); // Wait for updates
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
