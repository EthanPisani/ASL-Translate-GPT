#ifndef CHATGPTCLIENT_H
#define CHATGPTCLIENT_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class ChatGPTClient {
public:
    ChatGPTClient(const std::string& apiKey);
    std::string getCompletion(const std::string& prompt, const std::vector<nlohmann::json>& conversationHistory);

private:
    std::string apiKey;
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // CHATGPTCLIENT_H
