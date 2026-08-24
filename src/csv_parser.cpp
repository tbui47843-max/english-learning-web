#include "csv_parser.h"

#include <cctype>
#include <sstream>

std::vector<std::string> CsvParser::parseLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                field.push_back('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            fields.push_back(trim(field));
            field.clear();
        } else {
            field.push_back(ch);
        }
    }

    fields.push_back(trim(field));
    return fields;
}

std::string CsvParser::trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(start, end - start);
}

std::vector<VocabularyRecord> CsvParser::parseContent(const std::string& content) const {
    std::vector<VocabularyRecord> records;
    std::istringstream stream(content);
    std::string line;
    bool headerSkipped = false;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (!headerSkipped) {
            headerSkipped = true;
            continue;
        }

        const auto fields = parseLine(line);
        if (fields.size() < 2) {
            continue;
        }

        VocabularyRecord record;
        record.word = fields[0];
        record.meaning = fields[1];
        record.example = fields.size() > 2 ? fields[2] : "";
        record.pronunciation = fields.size() > 3 ? fields[3] : "";

        if (!record.word.empty() && !record.meaning.empty()) {
            records.push_back(record);
        }
    }

    return records;
}
