let currentExercise = null;

const promptEl = document.getElementById('translationPrompt');
const inputEl = document.getElementById('translationInput');
const resultEl = document.getElementById('translationResult');
const checkBtn = document.getElementById('checkTranslationBtn');
const nextBtn = document.getElementById('nextExerciseBtn');

async function loadExercise() {
  const response = await fetch('/api/translation/exercise');
  const payload = await response.json();

  if (!payload.available) {
    currentExercise = null;
    promptEl.textContent = 'Please study some flashcards first to unlock exercises.';
    inputEl.value = '';
    resultEl.textContent = '';
    return;
  }

  currentExercise = payload;
  promptEl.textContent = payload.vietnamesePrompt;
  inputEl.value = '';
  resultEl.textContent = '';
}

function normalize(text) {
  return text.toLowerCase().replace(/[^a-z0-9\s]/g, '').replace(/\s+/g, ' ').trim();
}

function checkTranslation() {
  if (!currentExercise) {
    resultEl.textContent = 'No active exercise.';
    return;
  }

  const answer = inputEl.value.trim();
  if (!answer) {
    resultEl.textContent = 'Please type your translation first.';
    return;
  }

  const normalizedAnswer = normalize(answer);
  const normalizedExpected = normalize(currentExercise.expectedAnswer || '');
  const normalizedTargetWord = normalize(currentExercise.targetWord || '');

  if (normalizedAnswer.includes(normalizedTargetWord) &&
      (normalizedExpected.length === 0 || normalizedAnswer === normalizedExpected)) {
    resultEl.textContent = '✅ Good job! Correct translation.';
  } else if (normalizedAnswer.includes(normalizedTargetWord)) {
    resultEl.textContent = `👍 You used the target word. Suggested answer: ${currentExercise.expectedAnswer}`;
  } else {
    resultEl.textContent = `❌ Try again. Make sure to use: ${currentExercise.targetWord}`;
  }
}

checkBtn.addEventListener('click', checkTranslation);
nextBtn.addEventListener('click', loadExercise);
