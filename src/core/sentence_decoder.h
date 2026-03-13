#pragma once

#include <memory>
#include <string>
#include <vector>

namespace core {

class Dictionary;
class LanguageModel;
class PinyinSegmenter;
struct CoreCandidate;

struct SentencePath {
    std::vector<std::string> words;
    float score = 0.0f;
};

class SentenceDecoder {
public:
    SentenceDecoder(std::shared_ptr<Dictionary> dict,
                    std::shared_ptr<LanguageModel> lm,
                    std::shared_ptr<PinyinSegmenter> segmenter);

    std::vector<SentencePath> decode(const std::string &pinyin,
                                     int beamWidth = 8) const;

    std::vector<CoreCandidate> decodeToCandidates(const std::string &pinyin,
                                                   int beamWidth = 8) const;

private:
    struct Node {
        std::vector<std::string> words;
        std::size_t pinyinPos;
        float score;
    };

    std::shared_ptr<Dictionary> dict_;
    std::shared_ptr<LanguageModel> lm_;
    std::shared_ptr<PinyinSegmenter> segmenter_;
};

} // namespace core
