// This file contains a class with a function that handles translating input text to any desired language.

#include "languageTranslate.h"
#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace std;
// Constructor includes the google api key we acquired/
languageTranslate::languageTranslate(): GOOGLE_API_KEY_("AIzaSyAOY2Vn_mMp3AdEtWo0EWWFN6mx4tUwkfk"){
}
    string languageTranslate::translate(const string& targetText, const string& selectedLanguage) {
        // Constructs the URL for making an HTTP request for translation.
        string url = "https://translation.googleapis.com/language/translate/v2?key=" + GOOGLE_API_KEY_;
        // Completes the URL using the target text (encoded) and selected language.
        url += "&q=" + cpr::util::urlEncode(targetText) + "&target=" + selectedLanguage;

        // Sends the HTTP GET request using the cpr library.
        cpr::Response response = cpr::Get(cpr::Url{url});

        // If HTTP response is successful.
        if (response.status_code == 200) {
            json responseJson = json::parse(response.text);

            // Accesses and stores the translatedText found in the translations array.
            const string translatedText = responseJson["data"]["translations"][0]["translatedText"].get<string>();
            return translatedText;
        } 
        // HTTP response unsuccessful.
        else {
            cerr << "An error occurred while attempting the HTTP request." << endl;
            return "[ERROR]: Translation Error";
        }
    }
