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
│   ├── main.cpp
│   ├── database.cpp/h
│   ├── csv_parser.cpp/h
│   └── vocabulary.cpp/h
├── public/
│   ├── index.html
│   ├── css/style.css
│   └── js/{app.js,flashcard.js,translation.js}
├── uploads/
├── CMakeLists.txt
├── README.md
└── sample_vocabulary.csv
```

## Build and Run

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
./english_learning_web
```

The app runs at: `http://localhost:18080`

## API Endpoints

- `POST /api/upload`: Upload CSV file content (text/plain)
- `GET /api/flashcard`: Get next flashcard
- `POST /api/progress`: Save `{ "wordId": number, "mastered": boolean }`
- `GET /api/translation/exercise`: Get translation exercise based on learned words

## CSV Format Example

Use `sample_vocabulary.csv` as reference.
