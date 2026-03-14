#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

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

    mutable std::list<std::string> decodeCacheOrder_;
    mutable std::unordered_map<std::string, std::vector<CoreCandidate>> decodeCache_;
    mutable std::list<std::string> segmentCacheOrder_;
    mutable std::unordered_map<std::string, std::string> segmentCache_;
    static constexpr std::size_t kDecodeCacheMaxSize = 128;
    static constexpr std::size_t kSegmentCacheMaxSize = 64;
};

} // namespace core
