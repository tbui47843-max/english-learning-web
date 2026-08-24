from pathlib import Path

from flask import Flask, jsonify, request, send_from_directory

from database import Database
from vocabulary import VocabularyManager

ROOT_DIR = Path(__file__).resolve().parent.parent
PUBLIC_DIR = ROOT_DIR / "public"
DB_PATH = ROOT_DIR / "english_learning.db"

app = Flask(__name__, static_folder=str(PUBLIC_DIR), static_url_path="")

database = Database(str(DB_PATH))
database.initialize()
vocabulary_manager = VocabularyManager(database)


@app.after_request
def add_cors_headers(response):
    response.headers["Access-Control-Allow-Origin"] = "*"
    response.headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS"
    response.headers["Access-Control-Allow-Headers"] = "Content-Type"
    return response


@app.get("/")
def index():
    return send_from_directory(PUBLIC_DIR, "index.html")


@app.route("/api/upload", methods=["POST", "OPTIONS"])
def upload():
    if request.method == "OPTIONS":
        return ("", 204)

    csv_content = request.get_data(as_text=True)
    if not csv_content and "file" in request.files:
        csv_content = request.files["file"].read().decode("utf-8", errors="ignore")

    imported = vocabulary_manager.import_from_csv(csv_content)
    status = 200 if imported > 0 else 400
    return jsonify({"imported": imported}), status


@app.get("/api/flashcard")
def flashcard():
    card = vocabulary_manager.next_flashcard()
    if card is None:
        return jsonify({"done": True})

    return jsonify(
        {
            "done": False,
            "id": card.id,
            "word": card.word,
            "meaning": card.meaning,
            "example": card.example,
            "pronunciation": card.pronunciation,
        }
    )


@app.route("/api/progress", methods=["POST", "OPTIONS"])
def progress():
    if request.method == "OPTIONS":
        return ("", 204)

    payload = request.get_json(silent=True) or {}
    if "wordId" not in payload or "mastered" not in payload:
        return ("", 400)

    saved = vocabulary_manager.save_progress(int(payload["wordId"]), bool(payload["mastered"]))
    return jsonify({"saved": saved}), (200 if saved else 500)


@app.get("/api/translation/exercise")
def translation_exercise():
    exercise = vocabulary_manager.next_exercise()
    if exercise is None:
        return jsonify({"available": False})

    return jsonify(
        {
            "available": True,
            "wordId": exercise.word_id,
            "vietnamesePrompt": exercise.vietnamese_prompt,
            "targetWord": exercise.target_word,
            "expectedAnswer": exercise.expected_answer,
        }
    )


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=18080)
