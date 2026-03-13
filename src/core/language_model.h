#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace core {

class Dictionary;

class LanguageModel {
public:
    explicit LanguageModel(std::shared_ptr<Dictionary> dict);

    float unigramScore(const std::string &word) const;

    float bigramScore(const std::string &prev, const std::string &current) const;

    float sentenceScore(const std::vector<std::string> &words) const;

    void addBigram(const std::string &prev, const std::string &current, float score);

private:
    std::shared_ptr<Dictionary> dict_;
    std::unordered_map<std::string, float> bigramScores_;
    float defaultScore_ = -10.0f;
};

} // namespace core
