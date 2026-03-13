#pragma once

#include <memory>
#include <string>
#include <vector>

namespace core {

struct CoreCandidate {
    std::string text;
    std::string comment;
};

class Decoder;

class InputContext {
public:
    InputContext();
    ~InputContext();

    void handleKeyEvent(int keySym, int keyState);

    std::string preeditText() const;
    std::vector<CoreCandidate> candidates() const;

    void commitCandidate(std::size_t index);

private:
    std::unique_ptr<Decoder> decoder_;
    std::string composingPinyin_;
    std::vector<CoreCandidate> currentCandidates_;
};

} // namespace core

