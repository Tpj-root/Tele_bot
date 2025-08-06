#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <unordered_map>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

unordered_map<int64_t, bool> loginRequested;
unordered_map<int64_t, bool> loggedIn;
unordered_map<int64_t, string> userResistorState;
unordered_map<int64_t, string> userPass = {
    {2102027453, "123"},
    {2102027454, "111"}
};

void createOneColumnKeyboard(const vector<string>& options, ReplyKeyboardMarkup::Ptr& keyboard) {
    keyboard->keyboard.clear();
    for (const auto& option : options) {
        KeyboardButton::Ptr button(new KeyboardButton);
        button->text = option;
        keyboard->keyboard.push_back({ button });
    }
    keyboard->resizeKeyboard = true;
    keyboard->oneTimeKeyboard = true;
}

void createKeyboard(const vector<vector<string>>& layout, ReplyKeyboardMarkup::Ptr& keyboard) {
    keyboard->keyboard.clear();
    for (const auto& row : layout) {
        vector<KeyboardButton::Ptr> buttonRow;
        for (const auto& label : row) {
            KeyboardButton::Ptr button(new KeyboardButton);
            button->text = label;
            buttonRow.push_back(button);
        }
        keyboard->keyboard.push_back(buttonRow);
    }
    keyboard->resizeKeyboard = true;
    keyboard->oneTimeKeyboard = true;
}

int main() {
    string token(getenv("TOKEN"));
    printf("Token: %s\n", token.c_str());

    Bot bot(token);

    // Setup commands
    vector<BotCommand::Ptr> commands;

    auto cmd = BotCommand::Ptr(new BotCommand);
    cmd->command = "start";
    cmd->description = "iam start";
    commands.push_back(cmd);

    cmd = BotCommand::Ptr(new BotCommand);
    cmd->command = "login";
    cmd->description = "login to account";
    commands.push_back(cmd);

    cmd = BotCommand::Ptr(new BotCommand);
    cmd->command = "logout";
    cmd->description = "logout";
    commands.push_back(cmd);

    cmd = BotCommand::Ptr(new BotCommand);
    cmd->command = "resistor";
    cmd->description = "resistor color code into value";
    commands.push_back(cmd);

    auto helpCommand = BotCommand::Ptr(new BotCommand);
    helpCommand->command = "help";
    helpCommand->description = "Display a list of all available commands.";
    commands.push_back(helpCommand);

    bot.getApi().setMyCommands(commands);



//Paste this inside any command or message handler where you want to restrict access only to logged-in users.
//
//Example for /resistor command (already correct):
//
//bot.getEvents().onCommand("resistor", [&bot](Message::Ptr message) {
//    if (!loggedIn[message->from->id]) return; // ✅ Restrict if not logged in
//
//    // continue logic...
//});


    // /start
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi! Use /login to access.");
    });

    // /help
    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
        string msg =
            "/start - I am start\n"
            "/login - Login to account\n"
            "/logout - Logout\n"
            "/resistor - Resistor color code into value\n"
            "/help - Display a list of all available commands.";
        bot.getApi().sendMessage(message->chat->id, msg);
    });

    // /resistor
    bot.getEvents().onCommand("resistor", [&bot](Message::Ptr message) {
        if (!loggedIn[message->from->id]) {
            bot.getApi().sendMessage(message->chat->id, "⛔ You must login first using /login.");
            return;
        }
        ReplyKeyboardMarkup::Ptr bandChoiceKeyboard(new ReplyKeyboardMarkup);
        createOneColumnKeyboard({ "4-band", "5-band" }, bandChoiceKeyboard);
        bot.getApi().sendMessage(message->chat->id, "Choose resistor type:", nullptr, nullptr, bandChoiceKeyboard);
        userResistorState[message->chat->id] = "awaiting_band_choice";
    });

    // resistor message handling
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        int64_t chatId = message->chat->id;
        string text = message->text;


        if (userResistorState[chatId] == "awaiting_band_choice") {
            if (text == "4-band" || text == "5-band") {
                userResistorState[chatId] = "awaiting_color_selection";

                ReplyKeyboardMarkup::Ptr colorKeyboard(new ReplyKeyboardMarkup);
                createKeyboard({
                    {"Black", "Brown", "Red"},
                    {"Orange", "Yellow", "Green"},
                    {"Blue", "Violet", "Gray"},
                    {"White", "Gold", "Silver"}
                }, colorKeyboard);

                bot.getApi().sendMessage(chatId, "Now, select the resistor color bands one by one:", nullptr, nullptr, colorKeyboard);
            } else {
                bot.getApi().sendMessage(chatId, "Please choose either '4-band' or '5-band'.");
            }
            return;
        }

        if (userResistorState[chatId] == "awaiting_color_selection") {
            bot.getApi().sendMessage(chatId, "You selected: " + text + "\n(Resistance calculation logic to be added)");
            return;
        }
    });

    // /login
    bot.getEvents().onCommand("login", [&bot](Message::Ptr message) {
        int64_t userId = message->from->id;
        string username = message->from->username;

        if (userPass.find(userId) == userPass.end()) {
            bot.getApi().sendMessage(message->chat->id,
                "🚫 You can't access this bot.\n"
                "Please contact admin.\n"
                "📞 Mob No: 98653624512");
            return;
        }

        loginRequested[userId] = true;
        bot.getApi().sendMessage(message->chat->id, "🔐 Type your password:");
    });

    // /logout
    bot.getEvents().onCommand("logout", [&bot](Message::Ptr message) {
        int64_t userId = message->from->id;
        string username = message->from->username;

        loggedIn[userId] = false;
        loginRequested[userId] = false;

        bot.getApi().sendMessage(message->chat->id, 
            "✅ You are logged out.\nBYE BYE @" + username + "!");
    });

    // login password handler
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        int64_t userId = message->from->id;
        string username = message->from->username;
        string text = message->text;

        if (StringTools::startsWith(text, "/")) return;

        if (loginRequested[userId]) {
            if (userPass[userId] == text) {
                loggedIn[userId] = true;
                loginRequested[userId] = false;
                bot.getApi().sendMessage(message->chat->id, "✅ Password correct.\nWelcome @" + username + "!");
            } else {
                bot.getApi().sendMessage(message->chat->id, "❌ Wrong password. Try again or use /login.");
            }
            return;
        }

        if (!loggedIn[userId]) {
            bot.getApi().sendMessage(message->chat->id, "⛔ You are not logged in.\nUse /login to continue.");
            return;
        }

        bot.getApi().sendMessage(message->chat->id, "📨 Message received: " + text);
    });

    // Ctrl+C handler
    signal(SIGINT, [](int s) {
        printf("SIGINT received\n");
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
