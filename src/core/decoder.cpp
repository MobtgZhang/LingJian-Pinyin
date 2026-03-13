#include "decoder.h"
#include "context.h"
#include "dictionary.h"
#include "language_model.h"
#include "pinyin_segmenter.h"
#include "sentence_decoder.h"

#include <algorithm>
#include <unordered_set>

namespace core {

Decoder::Decoder(std::shared_ptr<Dictionary> dict)
    : dict_(std::move(dict)),
      segmenter_(std::make_shared<PinyinSegmenter>()),
      lm_(std::make_shared<LanguageModel>(dict_)),
      sentenceDecoder_(
          std::make_unique<SentenceDecoder>(dict_, lm_, segmenter_)) {}

Decoder::~Decoder() = default;

/*
 * 解码流水线:
 *   拼音输入 → 拼音切分 → 拼音→汉字候选 → 语言模型评分
 *           → Beam Search → 输出候选句
 *
 * 1) 先用 SentenceDecoder 做整句解码（Beam Search）
 * 2) 再补充单音节精确匹配和前缀匹配的单字/单词候选
 * 3) 合并去重，保证最优结果在前
 */
std::vector<CoreCandidate> Decoder::decode(const std::string &pinyin) const {
    if (pinyin.empty() || !dict_ || !dict_->isLoaded()) {
        return {};
    }

    std::vector<CoreCandidate> result;
    std::unordered_set<std::string> seen;

    // 阶段1: 整句 Beam Search 解码
    auto sentenceCandidates = sentenceDecoder_->decodeToCandidates(pinyin, 10);
    for (auto &c : sentenceCandidates) {
        if (seen.insert(c.text).second) {
            result.push_back(std::move(c));
        }
    }

    // 阶段2: 传统单音节精确匹配（兜底）
    auto exact = dict_->lookup(pinyin);
    for (const auto &e : exact) {
        if (seen.insert(e.text).second) {
            result.push_back({e.text, e.pinyin});
        }
    }

    // 阶段3: 前缀匹配补充
    auto prefix = dict_->lookupPrefix(pinyin);
    for (const auto &e : prefix) {
        if (seen.insert(e.text).second) {
            result.push_back({e.text, e.pinyin});
        }
    }

    constexpr std::size_t kMaxCandidates = 100;
    if (result.size() > kMaxCandidates) {
        result.resize(kMaxCandidates);
    }

    return result;
}

std::string Decoder::segmentedPinyin(const std::string &pinyin) const {
    auto seg = segmenter_->bestSegment(pinyin);
    std::string result;
    for (std::size_t i = 0; i < seg.syllables.size(); ++i) {
        if (i > 0) result += "'";
        result += seg.syllables[i];
    }
    if (!seg.remainder.empty()) {
        if (!result.empty()) result += "'";
        result += seg.remainder;
    }
    return result;
}

} // namespace core
