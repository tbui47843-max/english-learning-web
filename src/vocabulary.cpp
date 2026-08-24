#include "vocabulary.h"

VocabularyManager::VocabularyManager(Database& database) : database_(database) {
}

int VocabularyManager::importFromCsv(const std::string& csvContent) {
    const auto records = parser_.parseContent(csvContent);
    return database_.importVocabulary(records);
}

std::optional<FlashcardData> VocabularyManager::nextFlashcard() const {
    return database_.getNextFlashcard();
}

bool VocabularyManager::saveProgress(int wordId, bool mastered) const {
    return database_.updateProgress(wordId, mastered);
}

std::optional<TranslationExercise> VocabularyManager::nextExercise() const {
    return database_.getTranslationExercise();
}
