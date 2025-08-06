// Include standard signal handling for graceful exit (like Ctrl+C)
#include <csignal>

// Standard C I/O (printf, etc.)
#include <cstdio>

// Standard library for general utilities like exit()
#include <cstdlib>

// Exception handling support
#include <exception>

// For using std::string
#include <string>

// For using std::vector
#include <vector>

// Include the Telegram Bot API C++ library (TgBot)
#include <tgbot/tgbot.h>

// Use standard namespace to avoid writing std:: repeatedly
using namespace std;

// Use TgBot namespace to access Bot, Message, etc. directly
using namespace TgBot;

// -------------------------
// Your main function starts
// -------------------------
int main() {
    // Get the bot token from environment variable "TOKEN"
    // Example to set: export TOKEN="your_bot_token_here"
    string token(getenv("TOKEN"));
    printf("Token: %s\n", token.c_str()); // Just printing to confirm

    // Create the Telegram Bot object using the token
    Bot bot(token);

    // -------------------------------
    // Add Bot Command Descriptions
    // These are shown in Telegram UI when user types "/"
    // -------------------------------
    vector<BotCommand::Ptr> commands;

    // Create a new command object for "/start"
    BotCommand::Ptr cmdStart(new BotCommand);
    cmdStart->command = "start";
    cmdStart->description = "Start the bot and get a welcome message";

    // Add the command to the list
    commands.push_back(cmdStart);

    // Apply commands to the bot (optional, good for UX)
    bot.getApi().setMyCommands(commands);

    // -------------------------------
    // Set message handler for "/start"
    // -------------------------------
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "👋 Hello! Welcome to the bot.");
    });

    // ----------------------------------
    // Global handler for any text message
    // ----------------------------------
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("Received message: %s\n", message->text.c_str());

        // Optional reply for non-command messages
        if (!message->text.empty() && message->text[0] != '/') {
            bot.getApi().sendMessage(message->chat->id, "You said: " + message->text);
        }
    });

    // -------------------------------
    // Graceful shutdown on Ctrl+C
    // -------------------------------
    signal(SIGINT, [](int s) {
        printf("Bot shutting down (SIGINT)\n");
        exit(0);
    });

    // -------------------------------
    // Start the bot (blocking loop)
    // This keeps the bot running and listening for messages
    // -------------------------------
    try {
        printf("Bot running...\n");
        TgLongPoll longPoll(bot); // Long polling setup
        while (true) {
            longPoll.start(); // Block here, waiting for messages
        }
    } catch (const exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
    }

    return 0;
}
