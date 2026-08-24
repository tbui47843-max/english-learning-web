let currentWordId = null;
let currentWord = '';

const wordEl = document.getElementById('word');
const pronunciationEl = document.getElementById('pronunciation');
const meaningEl = document.getElementById('meaning');
const exampleEl = document.getElementById('example');

const speakBtn = document.getElementById('speakBtn');
const knowBtn = document.getElementById('knowBtn');
const reviewBtn = document.getElementById('reviewBtn');

async function loadFlashcard() {
  const response = await fetch('/api/flashcard');
  const card = await response.json();

  if (card.done) {
    currentWordId = null;
    currentWord = '';
    wordEl.textContent = 'Great! You have reviewed all available words.';
    pronunciationEl.textContent = '';
    meaningEl.textContent = '';
    exampleEl.textContent = '';
    return;
  }

  currentWordId = card.id;
  currentWord = card.word;
  wordEl.textContent = card.word;
  pronunciationEl.textContent = card.pronunciation ? `Pronunciation: ${card.pronunciation}` : '';
  meaningEl.textContent = `Meaning: ${card.meaning}`;
  exampleEl.textContent = card.example ? `Example: ${card.example}` : '';
}

async function saveProgress(mastered) {
  if (!currentWordId) {
    return;
  }

  await fetch('/api/progress', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({ wordId: currentWordId, mastered })
  });

  await loadFlashcard();
}

function speakCurrentWord() {
  if (!currentWord || typeof speechSynthesis === 'undefined') {
    return;
  }

  const utterance = new SpeechSynthesisUtterance(currentWord);
  utterance.lang = 'en-US';
  speechSynthesis.speak(utterance);
}

speakBtn.addEventListener('click', speakCurrentWord);
knowBtn.addEventListener('click', () => saveProgress(true));
reviewBtn.addEventListener('click', () => saveProgress(false));
