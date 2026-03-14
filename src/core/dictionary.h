#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace core {

struct DictEntry {
    std::string pinyin;
    std::string text;
    float freq = 0.0f;
};

class Dictionary {
public:
    Dictionary();
    ~Dictionary();

    bool loadFromFile(const std::string &path);

    std::vector<DictEntry> lookup(const std::string &pinyin,
                                   std::size_t maxResults = 30) const;

    std::vector<DictEntry> lookupPrefix(const std::string &prefix,
                                        std::size_t maxResults = 50) const;

    bool isLoaded() const { return loaded_; }
    std::size_t entryCount() const { return allEntries_.size(); }
    const std::vector<DictEntry> &allEntries() const { return allEntries_; }

private:
    std::unordered_map<std::string, std::vector<DictEntry>> exactMap_;
    std::vector<std::string> sortedPinyinKeys_;
    std::vector<DictEntry> allEntries_;
    bool loaded_ = false;
};

} // namespace core
