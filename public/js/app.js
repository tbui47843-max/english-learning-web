const uploadBtn = document.getElementById('uploadBtn');
const csvFileInput = document.getElementById('csvFile');
const uploadStatus = document.getElementById('uploadStatus');

const flashcardModeBtn = document.getElementById('flashcardModeBtn');
const translationModeBtn = document.getElementById('translationModeBtn');
const flashcardPanel = document.getElementById('flashcardPanel');
const translationPanel = document.getElementById('translationPanel');

uploadBtn.addEventListener('click', async () => {
  const file = csvFileInput.files?.[0];
  if (!file) {
    uploadStatus.textContent = 'Please select a CSV file.';
    return;
  }

  const content = await file.text();
  const response = await fetch('/api/upload', {
    method: 'POST',
    headers: {
      'Content-Type': 'text/plain;charset=utf-8'
    },
    body: content
  });

  const data = await response.json();
  if (response.ok) {
    uploadStatus.textContent = `Imported ${data.imported} words successfully.`;
    await loadFlashcard();
  } else {
    uploadStatus.textContent = 'Failed to import CSV. Check format.';
  }
});

flashcardModeBtn.addEventListener('click', () => {
  flashcardPanel.classList.remove('hidden');
  translationPanel.classList.add('hidden');
});

translationModeBtn.addEventListener('click', async () => {
  translationPanel.classList.remove('hidden');
  flashcardPanel.classList.add('hidden');
  await loadExercise();
});

loadFlashcard();
