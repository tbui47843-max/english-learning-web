#ifndef DATABASE_H
#define DATABASE_H

#include "csv_parser.h"

#include <optional>
#include <sqlite3.h>
#include <string>
#include <vector>

struct FlashcardData {
    int id;
    std::string word;
    std::string meaning;
    std::string example;
    std::string pronunciation;
};

struct TranslationExercise {
    int wordId;
    std::string vietnamesePrompt;
    std::string targetWord;
    std::string expectedAnswer;
};

class Database {
public:
    explicit Database(const std::string& dbPath);
    ~Database();

    bool initialize();
    int importVocabulary(const std::vector<VocabularyRecord>& records);
    std::optional<FlashcardData> getNextFlashcard() const;
    bool updateProgress(int wordId, bool mastered) const;
    std::optional<TranslationExercise> getTranslationExercise() const;

private:
    sqlite3* db_;
    std::string dbPath_;

    bool exec(const std::string& sql) const;
};

#endif
