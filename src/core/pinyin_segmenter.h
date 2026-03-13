#pragma once

#include <string>
#include <vector>

namespace core {

class PinyinSegmenter {
public:
    PinyinSegmenter();

    struct SegmentResult {
        std::vector<std::string> syllables;
        std::string remainder;
    };

    std::vector<SegmentResult> segment(const std::string &input) const;

    SegmentResult bestSegment(const std::string &input) const;

    static bool isValidSyllable(const std::string &s);

private:
    void dfs(const std::string &input, std::size_t pos,
             std::vector<std::string> &current,
             std::vector<SegmentResult> &results) const;

    static const std::vector<std::string> &validSyllables();
};

} // namespace core
