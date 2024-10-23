#include <gtest/gtest.h>
#include "languageTranslate.h"

class LanguageTranslateTest : public ::testing::Test {
protected:
    languageTranslate translator;

    void TearDown() override {
        // Clean up after the tests, if necessary
    }
};

TEST_F(LanguageTranslateTest, BasicTranslation) {
    // Test basic translation
    std::string inputText = "Hello, world!";
    std::string translatedText = translator.translate(inputText, "es");  // Translate to Spanish
    EXPECT_EQ(translatedText, "¡Hola, mundo!");
}

TEST_F(LanguageTranslateTest, TranslationOfEmptyString) {
    // Test translation of an empty string
    std::string inputText = "";
    std::string translatedText = translator.translate(inputText, "fr");  // Translate to French
    EXPECT_EQ(translatedText, "");
}


TEST_F(LanguageTranslateTest, InvalidLanguageCode) {
    // Test with an invalid language code
    std::string inputText = "This is another test";
    std::string translatedText = translator.translate(inputText, "invalid_language");  // Invalid language code
    EXPECT_EQ(translatedText, "[ERROR]: Translation Error");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
