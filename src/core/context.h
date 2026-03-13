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
class Dictionary;

class InputContext {
public:
    InputContext();
    ~InputContext();

    bool loadDictionary(const std::string &path);

    enum class KeyResult {
        Consumed,
        Ignored,
        Committed,
    };

    KeyResult handleKey(char ch);
    KeyResult handleBackspace();
    KeyResult handleEscape();
    KeyResult handleEnter();
    KeyResult selectCandidate(std::size_t index);

    KeyResult handlePageDown();
    KeyResult handlePageUp();
    void handleCursorLeft();
    void handleCursorRight();
    int currentCursorIndex() const { return cursorIndex_; }

    std::string preeditText() const;
    std::string segmentedPreedit() const;
    std::vector<CoreCandidate> candidates() const;
    std::string committedText() const;

    bool isComposing() const { return !composingPinyin_.empty(); }
    void clearCommitted() { committedText_.clear(); }

    int currentPage() const { return currentPage_; }
    int totalPages() const;
    int pageSize() const { return pageSize_; }
    std::vector<CoreCandidate> currentPageCandidates() const;

private:
    void updateCandidates();

    std::shared_ptr<Dictionary> dict_;
    std::unique_ptr<Decoder> decoder_;
    std::string composingPinyin_;
    std::vector<CoreCandidate> allCandidates_;
    std::string committedText_;
    int currentPage_ = 0;
    int cursorIndex_ = 0;
    int pageSize_ = 5;
};

} // namespace core
