#include <iostream>
#include <vector>
#include "ChatGPTClient.h"
#include <curl/curl.h>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;
#include "InferenceYolov8.h"
#include "languageTranslate.h"


bool test_translate(){
    languageTranslate translator;
    std::string inputTextSpan = "Hello";
    std::string translatedTextSpan = translator.translate(inputTextSpan, "es");  // Translate to Spanish
    std::string translatedSpan = "Hola";

    std::string inputTextFr = "Hello";
    std::string translatedTextFr = translator.translate(inputTextFr, "fr");
    std::string translatedFr = "Bonjour";



    if (translatedTextSpan == translatedSpan && translatedTextFr == translatedFr) {
        return true;
    }
    else {
        return false;
    }
}

bool test_detect_2() {
    // test detect
    double timeLimit = 5.0; // Time limit in seconds

    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();
    // Create an instance of the OnnxYoloInfer class
    OnnxYoloInfer yolo;

    // Load image
    cv::Mat image = cv::imread("images/b1.jpg",1);

    // Perform detection
    std::vector<std::string> result = yolo.simple_detect(image);

    // Stop the timer
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Check if the detection took less time than the timeLimit
    if (duration.count() <= timeLimit) {
        return true;
    } else {
        return false;
    }
}

bool test_chatbot() {
    // Replace with your actual API key
    ChatGPTClient chatClient("API_KEY");
    string prompt = "Hello";
    vector<json> conversationHistory;

    // Add user's message to the conversation history
    conversationHistory.push_back({{"role", "user"}, {"content", prompt}});

    try {
        // Call getCompletion on the chatClient instance
        string response = chatClient.getCompletion(prompt, conversationHistory);

        // Add ChatGPT's message to the conversation history
        conversationHistory.push_back({{"role", "assistant"}, {"content", response}});

        // Check that the response is not empty
        if (response.empty()) {
            cerr << "ChatGPT response is empty." << endl;
            return false;
        }
    } catch (const std::exception& e) {
        cerr << "Error during chatbot test: " << e.what() << endl;
        return false;
    }

    return true;
}

bool test_detect_1(){
    // test detect
    OnnxYoloInfer yolo;

    // Load image
    cv::Mat image = cv::imread("images/q1.jpg");
    

    // simple detect
    std::vector<std::string> result = yolo.simple_detect(image);

    
    // assert result is q
    if (result[0] == "q"){
        return true;
    }
    else{
        return false;
    }
}

void translate(){
    // ask for input text
    string input_text;
    cout << "Please enter the text you want to translate: ";
    cin >> input_text;
    getline(cin, input_text);
    // ask for desired language
    string desired_language;
    cout << "Please enter the desired language: ";
    // list of valid google translate languages
    cout << "List of valid languages: " << endl;
    cout << "English: en" << endl;
    cout << "Spanish: es" << endl;
    cout << "French: fr" << endl;
    cout << "Select a language: ";
    cin >> desired_language;
    // translate
    languageTranslate translator;
    string translated_text = translator.translate(input_text, desired_language);
    // print translated text
    cout << "Translated text: " << translated_text << endl;

}

void chatbot(){

    // Create an instance of the ChatGPTClient
    ChatGPTClient chatClient("API_KEY"); // Replace with your actual API key
    string prompt;
    vector<json> conversationHistory;

    cout << "Welcome to the ChatGPT conversation interface!" << endl;
    cout << "You can start the conversation. Type 'exit' to end the session." << endl;

    try {
        while (true) {
            cout << "You: ";
            getline(cin, prompt);

            // Check if the user wants to exit
            if (prompt == "exit") {
                break;
            }

            // Add user's message to the conversation history
            conversationHistory.push_back({{"role", "user"}, {"content", prompt}});

            // Call getCompletion on the chatClient instance
            string response = chatClient.getCompletion(prompt, conversationHistory);

            cout << "ChatGPT: " << response << endl;

            // Add ChatGPT's message to the conversation history
            conversationHistory.push_back({{"role", "assistant"}, {"content", response}});
        }
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    
}

void detect(){
    // ask for image path
    string image_path;
    cout << "Please enter the path to the image: ";
    cin >> image_path;

    // validate image path
    // if invalid, ask for image path again
    while (true){
        if (image_path == "exit"){
            break;
        }
        else if (image_path == "help"){
            cout << "Please enter the path to the image: ";
            cin >> image_path;
        }
        else{
            break;
        }
    }

    // detect objects in image

    OnnxYoloInfer yolo;

    // Load image
    cv::Mat image = cv::imread(image_path);
    

    // simple detect
    std::vector<std::string> result = yolo.simple_detect(image);

    // print result
    for (int i = 0; i < result.size(); i++){
        cout << result[i] << endl;
    }
}

void all(){
    printf("Welcome to the AI interface!\n");
}

void test(){
    // call all test functions, print pass or fail
    bool detect_result_1 = test_detect_1();
    if (detect_result_1){
        cout << "Detect 1 test passed!" << endl;
    }
    else{
        cout << "Detect 1 test failed!" << endl;
    }
    bool detect_result_2 = test_detect_2();
    if (detect_result_2){
        cout << "Detect 2 test passed!" << endl;
    }
    else{
        cout << "Detect 2 test failed!" << endl;
    }

    bool chatbot_result = test_chatbot();
    if (chatbot_result){
        cout << "Chatbot test passed!" << endl;
    }
    else{
        cout << "Chatbot test failed!" << endl;
    }

    bool translate_result = test_translate();
    if (translate_result){
        cout << "Translate test passed!" << endl;
    }
    else{
        cout << "Translate test failed!" << endl;
    }
}

int main(int argc,char *argv[]){

    // print options (detect with yolo, talk to chatbot, translate, all)
    // get input from user
    int choice;
    std::cout << "Welcome to the AI interface!\n";
    std::cout << "Please select an option:\n";
    std::cout << "1. Detect with YOLOv8\n";
    std::cout << "2. Talk to Chatbot\n";
    std::cout << "3. Translate\n";
    std::cout << "4. All\n";
    std::cout << "5. Test\n";
    std::cout << "6. Exit\n";

    std::cout << "Enter your choice: ";

    while (true) {

        std::cin >> choice;

        if (choice >= 1 && choice <= 6) {
            // Valid choice, exit the loop
            break;
        } else {
            // clear the error state and ignore the input buffer
            std::cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid choice. Please enter a valid choice (1-6): ";
        }
    }


    // detect with yolo
    switch (choice)
    {
    case 1:
        detect();
        break;
    case 2:
        chatbot();
        break;
    case 3:
        translate();
        break;
    case 4:
        all();
        break;
    case 5:
        test();
        break;
    case 6:
        printf("Thank you for using our AI interface!\n");
        break;
    default:
        break;
    }
    return 0;

}
