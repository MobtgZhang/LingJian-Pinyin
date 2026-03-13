#include "dictionary.h"

namespace core {

Dictionary::Dictionary() = default;
Dictionary::~Dictionary() = default;

std::vector<DictEntry> Dictionary::lookup(const std::string &pinyin) const {
    std::vector<DictEntry> result;

    if (pinyin == "nihao") {
        result.push_back(DictEntry{"nihao", "你好", 1.0f});
    }

    return result;
}

} // namespace core

