#include "dictionary.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace core {

Dictionary::Dictionary() = default;
Dictionary::~Dictionary() = default;

bool Dictionary::loadFromFile(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    exactMap_.clear();
    allEntries_.clear();

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string pinyin, word;
        float freq = 0.0f;

        if (!(iss >> pinyin >> word)) {
            continue;
        }
        iss >> freq;

        DictEntry entry{pinyin, word, freq};
        exactMap_[pinyin].push_back(entry);
        allEntries_.push_back(std::move(entry));
    }

    for (auto &[key, entries] : exactMap_) {
        std::sort(entries.begin(), entries.end(),
                  [](const DictEntry &a, const DictEntry &b) {
                      return a.freq > b.freq;
                  });
    }

    loaded_ = true;
    return true;
}

std::vector<DictEntry> Dictionary::lookup(const std::string &pinyin) const {
    auto it = exactMap_.find(pinyin);
    if (it != exactMap_.end()) {
        return it->second;
    }
    return {};
}

std::vector<DictEntry> Dictionary::lookupPrefix(const std::string &prefix) const {
    if (prefix.empty()) {
        return {};
    }

    std::vector<DictEntry> result;
    for (const auto &[key, entries] : exactMap_) {
        if (key.size() >= prefix.size() &&
            key.compare(0, prefix.size(), prefix) == 0) {
            for (const auto &e : entries) {
                result.push_back(e);
            }
        }
    }

    std::sort(result.begin(), result.end(),
              [&prefix](const DictEntry &a, const DictEntry &b) {
                  bool aExact = (a.pinyin == prefix);
                  bool bExact = (b.pinyin == prefix);
                  if (aExact != bExact) return aExact;
                  if (a.pinyin.size() != b.pinyin.size())
                      return a.pinyin.size() < b.pinyin.size();
                  return a.freq > b.freq;
              });

    return result;
}

} // namespace core
