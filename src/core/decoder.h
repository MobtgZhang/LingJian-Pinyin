#pragma once

#include "lru_cache.h"

#include <memory>
#include <string>
#include <vector>

namespace core {

struct CoreCandidate;
class Dictionary;
class PinyinSegmenter;
class LanguageModel;
class SentenceDecoder;

class Decoder {
public:
    explicit Decoder(std::shared_ptr<Dictionary> dict);
    ~Decoder();

    std::vector<CoreCandidate> decode(const std::string &pinyin) const;

    std::string segmentedPinyin(const std::string &pinyin) const;

    void warmup();

private:
    std::vector<CoreCandidate> decodeImpl(const std::string &pinyin) const;

    std::shared_ptr<Dictionary> dict_;
    std::shared_ptr<PinyinSegmenter> segmenter_;
    std::shared_ptr<LanguageModel> lm_;
    std::unique_ptr<SentenceDecoder> sentenceDecoder_;

    mutable LruCache<std::string, std::vector<CoreCandidate>> decodeCache_{128};
    mutable LruCache<std::string, std::string> segmentCache_{64};
};

} // namespace core
