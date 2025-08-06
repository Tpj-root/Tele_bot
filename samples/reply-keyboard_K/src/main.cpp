#include <csignal>      // For handling Ctrl+C (SIGINT)
#include <cstdio>       // For printf, fprintf
#include <cstdlib>      // For getenv, exit
#include <exception>    // For exception handling
#include <string>       // For std::string
#include <vector>       // For std::vector
#include <tgbot/tgbot.h> // Telegram Bot API

using namespace std;
using namespace TgBot;



// Function to create a one-column custom keyboard layout
void createOneColumnKeyboard(const vector<string>& buttonStrings, ReplyKeyboardMarkup::Ptr& kb)
{
  for (size_t i = 0; i < buttonStrings.size(); ++i) {
    vector<KeyboardButton::Ptr> row;                 // Each row contains one button
    KeyboardButton::Ptr button(new KeyboardButton);  // Create a new button
    button->text = buttonStrings[i];                 // Set button label
    row.push_back(button);                           // Add button to row
    kb->keyboard.push_back(row);                     // Add row to keyboard layout
  }
}


// Create custom keyboard with multi-row and multi-column layout
void createKeyboard(const vector<vector<string>>& buttonLayout, ReplyKeyboardMarkup::Ptr& kb)
{
  for (size_t i = 0; i < buttonLayout.size(); ++i) {
    vector<KeyboardButton::Ptr> row; // One row of buttons
    for (size_t j = 0; j < buttonLayout[i].size(); ++j) {
      KeyboardButton::Ptr button(new KeyboardButton); // New button
      button->text = buttonLayout[i][j];              // Set label
      row.push_back(button);                          // Add button to row
    }
    kb->keyboard.push_back(row); // Add row to keyboard
  }
}



int main() {
    string token(getenv("TOKEN")); // Get bot token from environment
    printf("Token: %s\n", token.c_str());

    Bot bot(token); // Create bot instance

    // Create one-column keyboard
    ReplyKeyboardMarkup::Ptr keyboardOneCol(new ReplyKeyboardMarkup);
    createOneColumnKeyboard({"Option 1", "Option 2", "Option 3"}, keyboardOneCol);

    // Create multi-layout keyboard
    ReplyKeyboardMarkup::Ptr keyboardWithLayout(new ReplyKeyboardMarkup);
    createKeyboard({
      {"Dog", "Cat", "Mouse"},
      {"Green", "White", "Red"},
      {"On", "Off"},
      {"Back"},
      {"Info", "About", "Map", "Etc"}
    }, keyboardWithLayout);

    // /start command: shows one-column keyboard
    bot.getEvents().onCommand("start", [&bot, &keyboardOneCol](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "/start for one column keyboard\n/layout for a more complex keyboard", nullptr, nullptr, keyboardOneCol);
    });

    // /layout command: shows complex layout keyboard
    bot.getEvents().onCommand("layout", [&bot, &keyboardWithLayout](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "/start for one column keyboard\n/layout for a more complex keyboard", nullptr, nullptr, keyboardWithLayout);
    });

    // Handle any text messages
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start") || StringTools::startsWith(message->text, "/layout")) {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });

    // Exit on Ctrl+C
    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook(); // Remove any existing webhook

        TgLongPoll longPoll(bot); // Set up long polling
        while (true) {
            printf("Long poll started\n");
            longPoll.start(); // Block and wait for updates
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
