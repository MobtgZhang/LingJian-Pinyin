#pragma once

#include <string>
#include <vector>

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

    std::vector<DictEntry> lookup(const std::string &pinyin) const;
};

} // namespace core

