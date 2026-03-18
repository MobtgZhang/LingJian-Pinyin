#include "decoder.h"
#include "context.h"
#include "dictionary.h"
#include "language_model.h"
#include "pinyin_segmenter.h"
#include "sentence_decoder.h"

#include <unordered_set>

namespace core {

Decoder::Decoder(std::shared_ptr<Dictionary> dict)
    : dict_(std::move(dict)),
      segmenter_(std::make_shared<PinyinSegmenter>()),
      lm_(std::make_shared<LanguageModel>(dict_)),
      sentenceDecoder_(
          std::make_unique<SentenceDecoder>(dict_, lm_, segmenter_)) {}

Decoder::~Decoder() = default;

void Decoder::warmup() {
    if (lm_) lm_->ensureWordFreqCache();
}

std::vector<CoreCandidate> Decoder::decode(const std::string &pinyin) const {
    if (pinyin.empty() || !dict_ || !dict_->isLoaded()) {
        return {};
    }
    if (auto cached = decodeCache_.get(pinyin)) {
        return *cached;
    }
    auto result = decodeImpl(pinyin);
    decodeCache_.put(pinyin, result);
    return result;
}

/*
 * 解码流水线:
 *   拼音输入 → 拼音切分 → 拼音→汉字候选 → 语言模型评分
 *           → Beam Search → 输出候选句
 *
 * 1) 整词/整句匹配（你好、你好我的宝宝 等）
 * 2) 补充每个音节的单字候选，支持逐字选择（选"你"再选"好"）
 * 3) 合并去重，保证最优结果在前
 */
std::vector<CoreCandidate> Decoder::decodeImpl(const std::string &pinyin) const {
    std::vector<CoreCandidate> result;
    std::unordered_set<std::string> seen;

    auto addCandidate = [&](const std::string &text, const std::string &comment) {
        if (seen.insert(text).second) result.push_back({text, comment});
    };

    // 切分拼音为音节，用于补充单字候选
    auto seg = segmenter_->bestSegment(pinyin);
    const auto &syllables = seg.syllables;

    // 短输入快速路径
    constexpr std::size_t kFastPathMaxLen = 10;
    if (pinyin.size() <= kFastPathMaxLen) {
        // 1) 整词精确匹配 + 前缀匹配
        auto exact = dict_->lookup(pinyin, 50);
        for (const auto &e : exact) addCandidate(e.text, e.pinyin);
        auto prefix = dict_->lookupPrefix(pinyin, 30);
        for (const auto &e : prefix) addCandidate(e.text, e.pinyin);

        // 2) 每个音节的单字候选，支持逐字选择（如 nihao -> 你/尼/好/号）
        constexpr std::size_t kCharsPerSyllable = 12;
        for (const auto &syl : syllables) {
            auto sylEntries = dict_->lookup(syl, kCharsPerSyllable);
            for (const auto &e : sylEntries) addCandidate(e.text, e.pinyin);
        }

        if (result.size() > 80) result.resize(80);
        return result;
    }

    // 长输入：整句 Beam Search + 单字补充
    constexpr int kBeamWidth = 4;
    auto sentenceCandidates =
        sentenceDecoder_->decodeToCandidates(pinyin, kBeamWidth);
    for (auto &c : sentenceCandidates) addCandidate(c.text, c.comment);

    auto exact = dict_->lookup(pinyin);
    for (const auto &e : exact) addCandidate(e.text, e.pinyin);

    auto prefix = dict_->lookupPrefix(pinyin, 50);
    for (const auto &e : prefix) addCandidate(e.text, e.pinyin);

    // 每个音节的单字候选
    constexpr std::size_t kCharsPerSyllable = 10;
    for (const auto &syl : syllables) {
        auto sylEntries = dict_->lookup(syl, kCharsPerSyllable);
        for (const auto &e : sylEntries) addCandidate(e.text, e.pinyin);
    }

    constexpr std::size_t kMaxCandidates = 120;
    if (result.size() > kMaxCandidates) result.resize(kMaxCandidates);

    return result;
}

std::string Decoder::segmentedPinyin(const std::string &pinyin) const {
    if (pinyin.empty()) return {};
    if (auto cached = segmentCache_.get(pinyin)) {
        return *cached;
    }
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
    segmentCache_.put(pinyin, result);
    return result;
}

} // namespace core
