from typing import Optional

from csv_parser import CsvParser
from database import Database, FlashcardData, TranslationExercise


class VocabularyManager:
    def __init__(self, database: Database) -> None:
        self.database = database
        self.parser = CsvParser()

    def import_from_csv(self, csv_content: str) -> int:
        records = self.parser.parse_content(csv_content)
        return self.database.import_vocabulary(records)

    def next_flashcard(self) -> Optional[FlashcardData]:
        return self.database.get_next_flashcard()

    def save_progress(self, word_id: int, mastered: bool) -> bool:
        return self.database.update_progress(word_id, mastered)

    def next_exercise(self) -> Optional[TranslationExercise]:
        return self.database.get_translation_exercise()
