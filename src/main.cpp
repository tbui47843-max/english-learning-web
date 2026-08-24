#include "database.h"
#include "vocabulary.h"

#include <crow.h>

#include <fstream>
#include <sstream>
#include <string>

namespace {
std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void addCors(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
}
} // namespace

int main() {
    Database database("english_learning.db");
    if (!database.initialize()) {
        return 1;
    }

    VocabularyManager vocabularyManager(database);
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        crow::response res(readFile("public/index.html"));
        res.set_header("Content-Type", "text/html; charset=utf-8");
        addCors(res);
        return res;
    });

    CROW_ROUTE(app, "/css/<string>")([](const std::string& filename) {
        crow::response res(readFile("public/css/" + filename));
        res.set_header("Content-Type", "text/css; charset=utf-8");
        addCors(res);
        return res;
    });

    CROW_ROUTE(app, "/js/<string>")([](const std::string& filename) {
        crow::response res(readFile("public/js/" + filename));
        res.set_header("Content-Type", "application/javascript; charset=utf-8");
        addCors(res);
        return res;
    });

    CROW_ROUTE(app, "/api/upload").methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::POST)(
        [&vocabularyManager](const crow::request& req) {
            if (req.method == crow::HTTPMethod::OPTIONS) {
                crow::response res(204);
                addCors(res);
                return res;
            }

            const int imported = vocabularyManager.importFromCsv(req.body);
            crow::json::wvalue result;
            result["imported"] = imported;

            crow::response res(imported > 0 ? 200 : 400);
            res.set_header("Content-Type", "application/json");
            addCors(res);
            res.body = result.dump();
            return res;
        });

    CROW_ROUTE(app, "/api/flashcard").methods(crow::HTTPMethod::GET)([&vocabularyManager]() {
        const auto flashcard = vocabularyManager.nextFlashcard();
        crow::json::wvalue result;

        if (!flashcard.has_value()) {
            result["done"] = true;
        } else {
            result["done"] = false;
            result["id"] = flashcard->id;
            result["word"] = flashcard->word;
            result["meaning"] = flashcard->meaning;
            result["example"] = flashcard->example;
            result["pronunciation"] = flashcard->pronunciation;
        }

        crow::response res(result.dump());
        res.set_header("Content-Type", "application/json");
        addCors(res);
        return res;
    });

    CROW_ROUTE(app, "/api/progress").methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::POST)(
        [&vocabularyManager](const crow::request& req) {
            if (req.method == crow::HTTPMethod::OPTIONS) {
                crow::response res(204);
                addCors(res);
                return res;
            }

            const auto payload = crow::json::load(req.body);
            if (!payload || !payload.has("wordId") || !payload.has("mastered")) {
                crow::response res(400);
                addCors(res);
                return res;
            }

            const int wordId = payload["wordId"].i();
            const bool mastered = payload["mastered"].b();
            const bool saved = vocabularyManager.saveProgress(wordId, mastered);

            crow::json::wvalue result;
            result["saved"] = saved;

            crow::response res(saved ? 200 : 500);
            res.set_header("Content-Type", "application/json");
            addCors(res);
            res.body = result.dump();
            return res;
        });

    CROW_ROUTE(app, "/api/translation/exercise").methods(crow::HTTPMethod::GET)([&vocabularyManager]() {
        const auto exercise = vocabularyManager.nextExercise();
        crow::json::wvalue result;

        if (!exercise.has_value()) {
            result["available"] = false;
        } else {
            result["available"] = true;
            result["wordId"] = exercise->wordId;
            result["vietnamesePrompt"] = exercise->vietnamesePrompt;
            result["targetWord"] = exercise->targetWord;
            result["expectedAnswer"] = exercise->expectedAnswer;
        }

        crow::response res(result.dump());
        res.set_header("Content-Type", "application/json");
        addCors(res);
        return res;
    });

    app.port(18080).multithreaded().run();
    return 0;
}
