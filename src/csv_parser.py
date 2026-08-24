import csv
import io
from dataclasses import dataclass
from typing import List


@dataclass
class VocabularyRecord:
    word: str
    meaning: str
    example: str
    pronunciation: str


class CsvParser:
    def parse_content(self, content: str) -> List[VocabularyRecord]:
        records: List[VocabularyRecord] = []
        reader = csv.reader(io.StringIO(content))

        header_skipped = False
        for row in reader:
            if not header_skipped:
                header_skipped = True
                continue

            if not row:
                continue

            normalized = [cell.strip() for cell in row]
            if len(normalized) < 2:
                continue

            word = normalized[0]
            meaning = normalized[1]
            example = normalized[2] if len(normalized) > 2 else ""
            pronunciation = normalized[3] if len(normalized) > 3 else ""

            if word and meaning:
                records.append(
                    VocabularyRecord(
                        word=word,
                        meaning=meaning,
                        example=example,
                        pronunciation=pronunciation,
                    )
                )

        return records
