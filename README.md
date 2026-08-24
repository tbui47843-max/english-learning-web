# english-learning-web

Web application for learning English vocabulary with flashcards and translation exercises.

## Features

- Upload vocabulary from CSV files (`word, meaning, example, pronunciation`)
- Flashcard mode with pronunciation and meaning display
- Translation exercise mode using recently learned vocabulary
- SQLite progress tracking for learned/mastered words
- Responsive UI for desktop and mobile

## Project Structure

```
english-learning-web/
├── src/
│   ├── main.py
│   ├── database.py
│   ├── csv_parser.py
│   └── vocabulary.py
├── public/
│   ├── index.html
│   ├── css/style.css
│   └── js/{app.js,flashcard.js,translation.js}
├── uploads/
├── requirements.txt
├── README.md
└── sample_vocabulary.csv
```

## Run with Python

```bash
cd /home/runner/work/english-learning-web/english-learning-web
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python src/main.py
```

The app runs at: `http://localhost:18080`

## API Endpoints

- `POST /api/upload`: Upload CSV file content (text/plain)
- `GET /api/flashcard`: Get next flashcard
- `POST /api/progress`: Save `{ "wordId": number, "mastered": boolean }`
- `GET /api/translation/exercise`: Get translation exercise based on learned words

## CSV Format Example

Use `sample_vocabulary.csv` as reference.
