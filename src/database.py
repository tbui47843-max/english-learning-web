import sqlite3
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

from csv_parser import VocabularyRecord


@dataclass
class FlashcardData:
    id: int
    word: str
    meaning: str
    example: str
    pronunciation: str


@dataclass
class TranslationExercise:
    word_id: int
    vietnamese_prompt: str
    target_word: str
    expected_answer: str


class Database:
    def __init__(self, db_path: str) -> None:
        self.db_path = db_path
        self.conn: Optional[sqlite3.Connection] = None

    def initialize(self) -> None:
        Path(self.db_path).parent.mkdir(parents=True, exist_ok=True)
        self.conn = sqlite3.connect(self.db_path, check_same_thread=False)
        self.conn.execute("PRAGMA foreign_keys = ON")
        self.conn.executescript(
            """
            CREATE TABLE IF NOT EXISTS vocabulary (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word TEXT UNIQUE NOT NULL,
                meaning TEXT NOT NULL,
                example TEXT,
                pronunciation TEXT
            );
            CREATE TABLE IF NOT EXISTS progress (
                word_id INTEGER PRIMARY KEY,
                learned_count INTEGER NOT NULL DEFAULT 0,
                mastered INTEGER NOT NULL DEFAULT 0,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(word_id) REFERENCES vocabulary(id) ON DELETE CASCADE
            );
            """
        )
        self.conn.commit()

    def import_vocabulary(self, records: List[VocabularyRecord]) -> int:
        if self.conn is None:
            return 0

        count = 0
        with self.conn:
            for record in records:
                self.conn.execute(
                    """
                    INSERT INTO vocabulary(word, meaning, example, pronunciation)
                    VALUES(?, ?, ?, ?)
                    ON CONFLICT(word) DO UPDATE SET
                        meaning=excluded.meaning,
                        example=excluded.example,
                        pronunciation=excluded.pronunciation
                    """,
                    (record.word, record.meaning, record.example, record.pronunciation),
                )
                count += 1

            self.conn.execute("INSERT OR IGNORE INTO progress(word_id) SELECT id FROM vocabulary")

        return count

    def get_next_flashcard(self) -> Optional[FlashcardData]:
        if self.conn is None:
            return None

        row = self.conn.execute(
            """
            SELECT v.id, v.word, v.meaning, COALESCE(v.example, ''), COALESCE(v.pronunciation, '')
            FROM vocabulary v
            LEFT JOIN progress p ON p.word_id = v.id
            WHERE COALESCE(p.mastered, 0) = 0
            ORDER BY COALESCE(p.learned_count, 0) ASC, v.id ASC
            LIMIT 1
            """
        ).fetchone()

        if row is None:
            return None

        return FlashcardData(id=row[0], word=row[1], meaning=row[2], example=row[3], pronunciation=row[4])

    def update_progress(self, word_id: int, mastered: bool) -> bool:
        if self.conn is None:
            return False

        with self.conn:
            self.conn.execute(
                """
                INSERT INTO progress(word_id, learned_count, mastered, updated_at)
                VALUES(?, 1, ?, CURRENT_TIMESTAMP)
                ON CONFLICT(word_id) DO UPDATE SET
                    learned_count = learned_count + 1,
                    mastered = excluded.mastered,
                    updated_at = CURRENT_TIMESTAMP
                """,
                (word_id, 1 if mastered else 0),
            )
        return True

    def get_translation_exercise(self) -> Optional[TranslationExercise]:
        if self.conn is None:
            return None

        row = self.conn.execute(
            """
            SELECT v.id, v.word, v.meaning, COALESCE(v.example, '')
            FROM vocabulary v
            JOIN progress p ON p.word_id = v.id
            WHERE p.learned_count > 0
            ORDER BY p.updated_at DESC
            LIMIT 1
            """
        ).fetchone()

        if row is None:
            return None

        word_id, word, meaning, example = row
        prompt = f"Dịch sang tiếng Anh và dùng từ '{word}': Tôi đang ôn từ có nghĩa là '{meaning}'."
        expected = example if example else f"I am reviewing a word that means {meaning}."

        return TranslationExercise(
            word_id=word_id,
            vietnamese_prompt=prompt,
            target_word=word,
            expected_answer=expected,
        )
