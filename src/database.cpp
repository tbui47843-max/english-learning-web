#include "database.h"

#include <iostream>

Database::Database(const std::string& dbPath) : db_(nullptr), dbPath_(dbPath) {
}

Database::~Database() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::initialize() {
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const std::string schema =
        "CREATE TABLE IF NOT EXISTS vocabulary ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "word TEXT UNIQUE NOT NULL,"
        "meaning TEXT NOT NULL,"
        "example TEXT,"
        "pronunciation TEXT"
        ");"
        "CREATE TABLE IF NOT EXISTS progress ("
        "word_id INTEGER PRIMARY KEY,"
        "learned_count INTEGER NOT NULL DEFAULT 0,"
        "mastered INTEGER NOT NULL DEFAULT 0,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(word_id) REFERENCES vocabulary(id) ON DELETE CASCADE"
        ");";

    return exec(schema);
}

bool Database::exec(const std::string& sql) const {
    char* error = nullptr;
    const int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite error: " << (error != nullptr ? error : "unknown") << std::endl;
        if (error != nullptr) {
            sqlite3_free(error);
        }
        return false;
    }
    return true;
}

int Database::importVocabulary(const std::vector<VocabularyRecord>& records) {
    if (!exec("BEGIN TRANSACTION;")) {
        return 0;
    }

    const char* upsertSql =
        "INSERT INTO vocabulary(word, meaning, example, pronunciation) VALUES(?, ?, ?, ?) "
        "ON CONFLICT(word) DO UPDATE SET "
        "meaning=excluded.meaning, example=excluded.example, pronunciation=excluded.pronunciation;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, upsertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        exec("ROLLBACK;");
        return 0;
    }

    int count = 0;
    for (const auto& record : records) {
        sqlite3_bind_text(stmt, 1, record.word.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, record.meaning.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, record.example.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, record.pronunciation.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            ++count;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);

    if (!exec("INSERT OR IGNORE INTO progress(word_id) SELECT id FROM vocabulary;")) {
        exec("ROLLBACK;");
        return 0;
    }

    if (!exec("COMMIT;")) {
        exec("ROLLBACK;");
        return 0;
    }

    return count;
}

std::optional<FlashcardData> Database::getNextFlashcard() const {
    const char* sql =
        "SELECT v.id, v.word, v.meaning, COALESCE(v.example, ''), COALESCE(v.pronunciation, '') "
        "FROM vocabulary v "
        "LEFT JOIN progress p ON p.word_id = v.id "
        "WHERE COALESCE(p.mastered, 0) = 0 "
        "ORDER BY COALESCE(p.learned_count, 0) ASC, v.id ASC LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    std::optional<FlashcardData> flashcard;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        flashcard = FlashcardData{
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))};
    }

    sqlite3_finalize(stmt);
    return flashcard;
}

bool Database::updateProgress(int wordId, bool mastered) const {
    const char* sql =
        "INSERT INTO progress(word_id, learned_count, mastered, updated_at) VALUES(?, 1, ?, CURRENT_TIMESTAMP) "
        "ON CONFLICT(word_id) DO UPDATE SET "
        "learned_count = learned_count + 1, mastered = excluded.mastered, updated_at = CURRENT_TIMESTAMP;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, wordId);
    sqlite3_bind_int(stmt, 2, mastered ? 1 : 0);

    const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

std::optional<TranslationExercise> Database::getTranslationExercise() const {
    const char* sql =
        "SELECT v.id, v.word, v.meaning, COALESCE(v.example, '') "
        "FROM vocabulary v "
        "JOIN progress p ON p.word_id = v.id "
        "WHERE p.learned_count > 0 "
        "ORDER BY p.updated_at DESC LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    std::optional<TranslationExercise> exercise;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const int wordId = sqlite3_column_int(stmt, 0);
        const std::string word = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const std::string meaning = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const std::string example = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        exercise = TranslationExercise{
            wordId,
            "Dịch sang tiếng Anh và dùng từ '" + word + "': Tôi đang ôn từ có nghĩa là '" + meaning + "'.",
            word,
            example.empty() ? "I am reviewing a word that means " + meaning + "." : example};
    }

    sqlite3_finalize(stmt);
    return exercise;
}
