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
    sortedPinyinKeys_.clear();
    allEntries_.clear();

    std::string line;
    allEntries_.reserve(1000000);
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

    sortedPinyinKeys_.reserve(exactMap_.size());
    for (const auto &[key, _] : exactMap_) {
        sortedPinyinKeys_.push_back(key);
    }
    std::sort(sortedPinyinKeys_.begin(), sortedPinyinKeys_.end());

    loaded_ = true;
    return true;
}

std::vector<DictEntry> Dictionary::lookup(const std::string &pinyin,
                                          std::size_t maxResults) const {
    auto it = exactMap_.find(pinyin);
    if (it == exactMap_.end()) return {};
    const auto &entries = it->second;
    if (entries.size() <= maxResults) return entries;
    return std::vector<DictEntry>(entries.begin(),
                                  entries.begin() + maxResults);
}

std::vector<DictEntry> Dictionary::lookupPrefix(const std::string &prefix,
                                                 std::size_t maxResults) const {
    if (prefix.empty()) {
        return {};
    }

    auto it = std::lower_bound(sortedPinyinKeys_.begin(),
                                sortedPinyinKeys_.end(), prefix);
    std::vector<DictEntry> result;
    result.reserve(std::min(maxResults, std::size_t(100)));

    constexpr std::size_t kMaxKeysToScan = 150;
    std::size_t keysScanned = 0;

    for (; it != sortedPinyinKeys_.end() && keysScanned < kMaxKeysToScan; ++it, ++keysScanned) {
        if (it->size() < prefix.size() ||
            it->compare(0, prefix.size(), prefix) != 0) {
            break;
        }
        const auto &entries = exactMap_.find(*it)->second;
        for (const auto &e : entries) {
            result.push_back(e);
            if (result.size() >= maxResults) break;
        }
        if (result.size() >= maxResults) break;
    }
    if (result.empty()) return result;

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
