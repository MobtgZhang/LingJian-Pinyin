#pragma once

#include <algorithm>
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
    struct WordLink {
        std::shared_ptr<WordLink> prev;
        std::string word;
    };

    struct Node {
        std::shared_ptr<WordLink> tail;
        std::size_t pinyinPos;
        float score;

        std::vector<std::string> collectWords() const {
            std::vector<std::string> result;
            for (auto p = tail; p; p = p->prev) result.push_back(p->word);
            std::reverse(result.begin(), result.end());
            return result;
        }

        std::string lastWord() const {
            return tail ? tail->word : std::string{};
        }
    };

    std::shared_ptr<Dictionary> dict_;
    std::shared_ptr<LanguageModel> lm_;
    std::shared_ptr<PinyinSegmenter> segmenter_;
};

} // namespace core
