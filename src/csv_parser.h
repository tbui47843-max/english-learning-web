#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>

struct VocabularyRecord {
    std::string word;
    std::string meaning;
    std::string example;
    std::string pronunciation;
};

class CsvParser {
public:
    std::vector<VocabularyRecord> parseContent(const std::string& content) const;

private:
    static std::vector<std::string> parseLine(const std::string& line);
    static std::string trim(const std::string& value);
};

#endif
