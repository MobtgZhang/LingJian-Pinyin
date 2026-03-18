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

    const std::string &preeditText() const { return composingPinyin_; }
    std::string segmentedPreedit() const;
    const std::vector<CoreCandidate> &candidates() const { return allCandidates_; }
    const std::string &committedText() const { return committedText_; }

    bool isComposing() const { return !composingPinyin_.empty(); }
    void clearCommitted() { committedText_.clear(); }

    int currentPage() const { return currentPage_; }
    int totalPages() const;
    int pageSize() const { return pageSize_; }
    std::vector<CoreCandidate> currentPageCandidates() const;

    /** 刷新候选列表（解码），由 UI 在防抖后或需要时调用 */
    void updateCandidates();

private:

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
