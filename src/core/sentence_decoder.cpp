#include "sentence_decoder.h"
#include "context.h"
#include "dictionary.h"
#include "language_model.h"
#include "pinyin_segmenter.h"

#include <algorithm>
#include <queue>
#include <unordered_set>

namespace core {

SentenceDecoder::SentenceDecoder(std::shared_ptr<Dictionary> dict,
                                  std::shared_ptr<LanguageModel> lm,
                                  std::shared_ptr<PinyinSegmenter> segmenter)
    : dict_(std::move(dict)), lm_(std::move(lm)),
      segmenter_(std::move(segmenter)) {}

namespace {
constexpr std::size_t kMaxEntriesPerLookup = 20;
constexpr std::size_t kMaxSpan = 3;
}

std::vector<SentencePath> SentenceDecoder::decode(
    const std::string &pinyin, int beamWidth) const {
    if (pinyin.empty() || !dict_ || !dict_->isLoaded()) return {};

    auto segResult = segmenter_->bestSegment(pinyin);
    const auto &syllables = segResult.syllables;
    if (syllables.empty()) return {};

    struct NodeCmp {
        bool operator()(const Node &a, const Node &b) const {
            return a.score < b.score;
        }
    };
    using BeamQueue = std::priority_queue<Node, std::vector<Node>, NodeCmp>;
    BeamQueue beam;

    beam.push({nullptr, 0, 0.0f});

    for (std::size_t step = 0; step < syllables.size(); ++step) {
        BeamQueue nextBeam;

        std::vector<Node> currentNodes;
        currentNodes.reserve(beamWidth);
        for (int i = 0; !beam.empty() && i < beamWidth; ++i) {
            currentNodes.push_back(std::move(const_cast<Node &>(beam.top())));
            beam.pop();
        }

        for (const auto &node : currentNodes) {
            if (node.pinyinPos != step) {
                nextBeam.push(node);
                continue;
            }

            std::unordered_set<std::string> seenWords;
            for (std::size_t span = 1;
                 span <= syllables.size() - step && span <= kMaxSpan; ++span) {
                std::string flatPinyin;
                for (std::size_t k = 0; k < span; ++k)
                    flatPinyin += syllables[step + k];

                auto tryEntries = [&](const std::string &py) {
                    auto entries = dict_->lookup(py, kMaxEntriesPerLookup);
                    for (const auto &e : entries) {
                        if (!seenWords.insert(e.text).second) continue;

                        float wordScore = node.tail
                            ? lm_->bigramScore(node.lastWord(), e.text)
                            : lm_->unigramScore(e.text);

                        auto link = std::make_shared<WordLink>(
                            WordLink{node.tail, e.text});
                        nextBeam.push({link, step + span,
                                       node.score + wordScore});
                    }
                };

                tryEntries(flatPinyin);
                if (span > 1) {
                    std::string combined;
                    for (std::size_t k = 0; k < span; ++k) {
                        if (k > 0) combined += "'";
                        combined += syllables[step + k];
                    }
                    if (combined != flatPinyin) tryEntries(combined);
                }
            }
        }

        beam = BeamQueue{};
        for (int i = 0; !nextBeam.empty() && i < beamWidth; ++i) {
            beam.push(std::move(const_cast<Node &>(nextBeam.top())));
            nextBeam.pop();
        }
    }

    std::vector<SentencePath> results;
    std::unordered_set<std::string> seenSentences;
    while (!beam.empty()) {
        auto node = std::move(const_cast<Node &>(beam.top()));
        beam.pop();
        if (node.pinyinPos != syllables.size()) continue;

        auto words = node.collectWords();
        std::string sentence;
        for (const auto &w : words) sentence += w;

        if (seenSentences.insert(sentence).second) {
            results.push_back({std::move(words), node.score});
        }
    }

    std::sort(results.begin(), results.end(),
              [](const SentencePath &a, const SentencePath &b) {
                  return a.score > b.score;
              });

    constexpr std::size_t kMaxResults = 20;
    if (results.size() > kMaxResults) results.resize(kMaxResults);
    return results;
}

std::vector<CoreCandidate> SentenceDecoder::decodeToCandidates(
    const std::string &pinyin, int beamWidth) const {
    auto paths = decode(pinyin, beamWidth);
    std::vector<CoreCandidate> candidates;
    candidates.reserve(paths.size());

    for (const auto &path : paths) {
        std::string text;
        for (const auto &w : path.words) text += w;

        std::string comment;
        for (std::size_t i = 0; i < path.words.size(); ++i) {
            if (i > 0) comment += " ";
            comment += path.words[i];
        }
        candidates.push_back({text, comment});
    }
    return candidates;
}

} // namespace core
