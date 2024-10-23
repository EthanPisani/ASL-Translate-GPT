#include <gtest/gtest.h>
#include "../src/ChatGPTClient.h"
#include <nlohmann/json.hpp>
#include <thread> 

using json = nlohmann::json;

class ChatGPTClientTest : public ::testing::Test {
protected:
    ChatGPTClient* client;

    virtual void SetUp() {
        // Initialize the client with your actual API key
        client = new ChatGPTClient("API_KEY");
    }

    virtual void TearDown() {
        delete client;
    }
};

TEST_F(ChatGPTClientTest, APIResponseIsNotEmpty) {
    std::vector<json> conversationHistory;
    std::string prompt = "Hello, how are you?";
    std::string response;
    int retries = 3; // Number of retries
    bool success = false;

    for (int i = 0; i < retries; ++i) {
        try {
            response = client->getCompletion(prompt, conversationHistory);
            if (!response.empty()) {
                success = true;
                break; // Response is not empty, break out of the loop
            }
            // Wait for a second before retrying
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } catch (const std::exception& e) {
            // Optionally handle the exception, e.g., print the error message
            std::cerr << "Exception thrown: " << e.what() << std::endl;
        }
    }

    EXPECT_TRUE(success) << "API call completed but the response is empty after " << retries << " retries.";
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
