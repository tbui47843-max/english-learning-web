#ifndef VOCABULARY_H
#define VOCABULARY_H

#include "csv_parser.h"
#include "database.h"

#include <optional>
#include <string>

class VocabularyManager {
public:
    explicit VocabularyManager(Database& database);

    int importFromCsv(const std::string& csvContent);
    std::optional<FlashcardData> nextFlashcard() const;
    bool saveProgress(int wordId, bool mastered) const;
    std::optional<TranslationExercise> nextExercise() const;

private:
    Database& database_;
    CsvParser parser_;
};

#endif
