#include "decoder.h"

#include "context.h"

namespace core {

Decoder::Decoder() = default;
Decoder::~Decoder() = default;

std::vector<CoreCandidate> Decoder::decode(const std::string &pinyin) const {
    std::vector<CoreCandidate> result;
    if (pinyin.empty()) {
        return result;
    }

    // 占位实现：简单地把拼音本身作为候选。
    CoreCandidate c;
    c.text = "[候选]" + pinyin;
    c.comment = "demo";
    result.push_back(c);

    return result;
}

} // namespace core

