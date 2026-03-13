#pragma once

#include <string>
#include <vector>

namespace core {

struct CoreCandidate;

class Decoder {
public:
    Decoder();
    ~Decoder();

    std::vector<CoreCandidate> decode(const std::string &pinyin) const;
};

} // namespace core

