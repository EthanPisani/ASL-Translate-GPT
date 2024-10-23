// This header file outlines the structure of the languageTranslate class. It provides the constructor as well as private and public sections and defines member variables and functions.

#ifndef LANGUAGE_TRANSLATE_H
#define LANGUAGE_TRANSLATE_H

#include <string>

class languageTranslate {
public:
    languageTranslate();  
    std::string translate(const std::string& targetText, const std::string& selectedLanguage);

private:
    const std::string GOOGLE_API_KEY_;
};

#endif