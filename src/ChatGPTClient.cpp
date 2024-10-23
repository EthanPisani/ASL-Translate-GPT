#include "ChatGPTClient.h"
#include <curl/curl.h>
#include <stdexcept>

using json = nlohmann::json;
using namespace std;

ChatGPTClient::ChatGPTClient(const std::string& apiKey) : apiKey(apiKey) {}

size_t ChatGPTClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Modify function signature to accept conversation history
string ChatGPTClient::getCompletion(const string& prompt, const vector<json>& conversationHistory) {
    string baseUrl = "https://api.openai.com/v1/chat/completions";
    string response;
    CURL* curl = curl_easy_init();

    if (curl) {
        json requestData = {
            {"model", "gpt-3.5-turbo"},
            {"messages", conversationHistory}
        };

        string requestDataStr = requestData.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        string authHeader = "Authorization: Bearer " + this->apiKey; // Use the apiKey member variable
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, baseUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestDataStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            throw runtime_error(curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        // Parse the response
        json responseJson = json::parse(response);
        string content = responseJson["choices"][0]["message"]["content"].get<string>();
        return content;
    } else {
        throw runtime_error("Failed to initialize cURL");
    }
}
