#include "language_model.h"
#include "dictionary.h"

#include <cmath>
#include <algorithm>

namespace core {

LanguageModel::LanguageModel(std::shared_ptr<Dictionary> dict)
    : dict_(std::move(dict)) {}

float LanguageModel::unigramScore(const std::string &word) const {
    if (!dict_ || !dict_->isLoaded()) return defaultScore_;

    float maxFreq = 0.0f;
    auto entries = dict_->allEntries();
    for (const auto &e : entries) {
        if (e.text == word) {
            maxFreq = std::max(maxFreq, e.freq);
        }
    }

    if (maxFreq <= 0.0f) return defaultScore_;
    return std::log10(maxFreq + 1.0f);
}

float LanguageModel::bigramScore(const std::string &prev,
                                  const std::string &current) const {
    std::string key = prev + "|" + current;
    auto it = bigramScores_.find(key);
    if (it != bigramScores_.end()) {
        return it->second;
    }
    return unigramScore(current);
}

float LanguageModel::sentenceScore(const std::vector<std::string> &words) const {
    if (words.empty()) return defaultScore_;

    float total = unigramScore(words[0]);
    for (std::size_t i = 1; i < words.size(); ++i) {
        total += bigramScore(words[i - 1], words[i]);
    }
    return total / static_cast<float>(words.size());
}

void LanguageModel::addBigram(const std::string &prev,
                               const std::string &current, float score) {
    bigramScores_[prev + "|" + current] = score;
}

} // namespace core
