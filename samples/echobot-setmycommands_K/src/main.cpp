#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <unordered_map>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

// Store login state per user
unordered_map<int64_t, bool> loginRequested;
unordered_map<int64_t, bool> loggedIn;
// Add this outside of main (global or shared scope)
std::unordered_map<int64_t, std::string> userResistorState;
// Hardcoded user_id → password map
unordered_map<int64_t, string> userPass = {
    {2102027453, "123"},
    {2102027454, "111"}
};


void createOneColumnKeyboard(const std::vector<std::string>& options, TgBot::ReplyKeyboardMarkup::Ptr& keyboard) {
    keyboard->keyboard.clear();
    for (const auto& option : options) {
        TgBot::KeyboardButton::Ptr button(new TgBot::KeyboardButton);
        button->text = option;
        keyboard->keyboard.push_back({ button });
    }
    keyboard->resizeKeyboard = true;
    keyboard->oneTimeKeyboard = true;
}

void createKeyboard(const std::vector<std::vector<std::string>>& layout, TgBot::ReplyKeyboardMarkup::Ptr& keyboard) {
    keyboard->keyboard.clear();
    for (const auto& row : layout) {
        std::vector<TgBot::KeyboardButton::Ptr> buttonRow;
        for (const auto& label : row) {
            TgBot::KeyboardButton::Ptr button(new TgBot::KeyboardButton);
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

    // Setup bot commands
    vector<BotCommand::Ptr> commands;
    BotCommand::Ptr cmdArray(new BotCommand);
    cmdArray->command = "start";
    cmdArray->description = "iam start";
    commands.push_back(cmdArray);

    cmdArray = BotCommand::Ptr(new BotCommand);
    cmdArray->command = "login";
    cmdArray->description = "login to account";
    commands.push_back(cmdArray);

    cmdArray = BotCommand::Ptr(new BotCommand);
    cmdArray->command = "logout";
    cmdArray->description = "logout";
    commands.push_back(cmdArray);

    cmdArray = BotCommand::Ptr(new BotCommand);
    cmdArray->command = "resistor";
    cmdArray->description = "resistor color code into value";
    commands.push_back(cmdArray);

	auto helpCommand = BotCommand::Ptr(new BotCommand);
	helpCommand->command = "help";
	helpCommand->description = "Display a list of all available commands.";
	commands.push_back(helpCommand);


    bot.getApi().setMyCommands(commands);

    // Command: /start
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi! Use /login to access.");
    });

    // Command: /start
    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
    	    std::string msg = 
    	"/start - I am start\n"
    	"/login - Login to account\n"
    	"/logout - Logout\n"
    	"/resistor - Resistor color code into value\n"
    	"/help - Display a list of all available commands.";

		bot.getApi().sendMessage(message->chat->id, msg);
    });

    // Command: /resistor
	// Store user state for resistor input
	// Inside your main function or setup
	bot.getEvents().onCommand("resistor", [&bot, &userResistorState](Message::Ptr message) {
	    TgBot::ReplyKeyboardMarkup::Ptr bandChoiceKeyboard(new TgBot::ReplyKeyboardMarkup);
	    createOneColumnKeyboard({"4-band", "5-band"}, bandChoiceKeyboard);
	    bot.getApi().sendMessage(message->chat->id, "Choose resistor type:", nullptr, nullptr, bandChoiceKeyboard);
	    userResistorState[message->chat->id] = "awaiting_band_choice";
	});
	
	bot.getEvents().onAnyMessage([&bot, &userResistorState](Message::Ptr message) {
	    int64_t chatId = message->chat->id;
	    std::string text = message->text;
	
	    if (userResistorState[chatId] == "awaiting_band_choice") {
	        if (text == "4-band" || text == "5-band") {
	            userResistorState[chatId] = "awaiting_color_selection";
	
	            TgBot::ReplyKeyboardMarkup::Ptr colorKeyboard(new TgBot::ReplyKeyboardMarkup);
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
	    }
	    else if (userResistorState[chatId] == "awaiting_color_selection") {
	        bot.getApi().sendMessage(chatId, "You selected: " + text + "\n(Resistance calculation logic to be added)");
	    }
	});


    // Command: /login
    bot.getEvents().onCommand("login", [&bot](Message::Ptr message) {
        int64_t userId = message->from->id;
        string username = message->from->username;

        // Check if user is allowed
        if (userPass.find(userId) == userPass.end()) {
            bot.getApi().sendMessage(message->chat->id,
                "🚫 You can't access this bot.\n"
                "Please contact admin.\n"
                "📞 Mob No: 98653624512");
            return;
        }

        // Ask for password
        loginRequested[userId] = true;
        bot.getApi().sendMessage(message->chat->id, "🔐 Type your password:");
    });

    // Command: /logout
    bot.getEvents().onCommand("logout", [&bot](Message::Ptr message) {
        int64_t userId = message->from->id;
        //Get the username 
        string username = message->from->username;
        loggedIn[userId] = false;
        loginRequested[userId] = false;
        bot.getApi().sendMessage(message->chat->id, 
        	"✅ You are logged out.\n"
            "BYE BYE @" + username + "!");
    });

    // Any message (for password input or normal message)
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        int64_t userId = message->from->id;
        string username = message->from->username;
        string text = message->text;

        printf("User wrote: %s\n", text.c_str());

        if (StringTools::startsWith(text, "/")) return;

        // If waiting for password
        if (loginRequested[userId]) {
            if (userPass[userId] == text) {
                loggedIn[userId] = true;
                loginRequested[userId] = false;

                bot.getApi().sendMessage(message->chat->id,
                    "✅ Password correct.\n"
                    "Welcome @" + username + "!");
            } else {
                bot.getApi().sendMessage(message->chat->id,
                    "❌ Wrong password. Try again or use /login.");
            }
            return;
        }

        // If not logged in
        if (!loggedIn[userId]) {
            bot.getApi().sendMessage(message->chat->id,
                "⛔ You are not logged in.\nUse /login to continue.");
            return;
        }

        // Normal message after login
        bot.getApi().sendMessage(message->chat->id,
            "📨 Message received: " + text);
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
