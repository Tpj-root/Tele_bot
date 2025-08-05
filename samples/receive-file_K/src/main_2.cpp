#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <fstream>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

int main() {
    string token(getenv("TOKEN"));
    printf("Token: %s\n", token.c_str());

    Bot bot(token);
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote: %s\n", message->text.c_str());

        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }

        if (message->document) {
            try {
                File::Ptr file = bot.getApi().getFile(message->document->fileId);
                string fileContent = bot.getApi().downloadFile(file->filePath);

                // Save to local file
                string localFileName = "downloaded_" + message->document->fileName;
                ofstream out(localFileName, ios::binary);
                out << fileContent;
                out.close();

                bot.getApi().sendMessage(message->chat->id, "✅ File saved as: " + localFileName);
            } catch (exception& e) {
                bot.getApi().sendMessage(message->chat->id, "❌ Error downloading/saving file");
                printf("Download error: %s\n", e.what());
            }
        }
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
