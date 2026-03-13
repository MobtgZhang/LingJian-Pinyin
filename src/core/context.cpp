#include "context.h"
#include "decoder.h"
#include "dictionary.h"

#include <algorithm>

namespace core {

InputContext::InputContext()
    : dict_(std::make_shared<Dictionary>()),
      decoder_(std::make_unique<Decoder>(dict_)) {}

InputContext::~InputContext() = default;

bool InputContext::loadDictionary(const std::string &path) {
    return dict_->loadFromFile(path);
}

InputContext::KeyResult InputContext::handleKey(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        composingPinyin_.push_back(ch);
        updateCandidates();
        return KeyResult::Consumed;
    }
    if (ch >= 'A' && ch <= 'Z') {
        composingPinyin_.push_back(static_cast<char>(ch - 'A' + 'a'));
        updateCandidates();
        return KeyResult::Consumed;
    }

    if (composingPinyin_.empty()) {
        return KeyResult::Ignored;
    }

    if (ch == ' ') {
        return selectCandidate(0);
    }

    if (ch >= '1' && ch <= '9') {
        int idx = (ch - '1') + currentPage_ * pageSize_;
        return selectCandidate(static_cast<std::size_t>(idx));
    }

    return KeyResult::Ignored;
}

InputContext::KeyResult InputContext::handleBackspace() {
    if (composingPinyin_.empty()) {
        return KeyResult::Ignored;
    }
    composingPinyin_.pop_back();
    updateCandidates();
    if (composingPinyin_.empty()) {
        return KeyResult::Committed;
    }
    return KeyResult::Consumed;
}

InputContext::KeyResult InputContext::handleEscape() {
    if (composingPinyin_.empty()) {
        return KeyResult::Ignored;
    }
    composingPinyin_.clear();
    allCandidates_.clear();
    currentPage_ = 0;
    return KeyResult::Consumed;
}

InputContext::KeyResult InputContext::handleEnter() {
    if (composingPinyin_.empty()) {
        return KeyResult::Ignored;
    }
    committedText_ = composingPinyin_;
    composingPinyin_.clear();
    allCandidates_.clear();
    currentPage_ = 0;
    return KeyResult::Committed;
}

InputContext::KeyResult InputContext::selectCandidate(std::size_t index) {
    if (index >= allCandidates_.size()) {
        if (!allCandidates_.empty()) {
            index = 0;
        } else {
            committedText_ = composingPinyin_;
            composingPinyin_.clear();
            allCandidates_.clear();
            currentPage_ = 0;
            return KeyResult::Committed;
        }
    }
    committedText_ = allCandidates_[index].text;
    composingPinyin_.clear();
    allCandidates_.clear();
    currentPage_ = 0;
    return KeyResult::Committed;
}

InputContext::KeyResult InputContext::handlePageDown() {
    if (currentPage_ < totalPages() - 1) {
        ++currentPage_;
        cursorIndex_ = 0;
        return KeyResult::Consumed;
    }
    return KeyResult::Ignored;
}

InputContext::KeyResult InputContext::handlePageUp() {
    if (currentPage_ > 0) {
        --currentPage_;
        cursorIndex_ = 0;
        return KeyResult::Consumed;
    }
    return KeyResult::Ignored;
}

std::string InputContext::preeditText() const {
    return composingPinyin_;
}

std::string InputContext::segmentedPreedit() const {
    if (composingPinyin_.empty()) return {};
    return decoder_->segmentedPinyin(composingPinyin_);
}

std::vector<CoreCandidate> InputContext::candidates() const {
    return allCandidates_;
}

std::string InputContext::committedText() const {
    return committedText_;
}

int InputContext::totalPages() const {
    if (allCandidates_.empty()) return 1;
    return static_cast<int>((allCandidates_.size() + pageSize_ - 1) / pageSize_);
}

std::vector<CoreCandidate> InputContext::currentPageCandidates() const {
    std::vector<CoreCandidate> page;
    int start = currentPage_ * pageSize_;
    int end = std::min(start + pageSize_,
                       static_cast<int>(allCandidates_.size()));
    for (int i = start; i < end; ++i) {
        page.push_back(allCandidates_[i]);
    }
    return page;
}

void InputContext::updateCandidates() {
    currentPage_ = 0;
    cursorIndex_ = 0;
    if (composingPinyin_.empty()) {
        allCandidates_.clear();
        return;
    }
    allCandidates_ = decoder_->decode(composingPinyin_);
}

void InputContext::handleCursorLeft() {
    if (cursorIndex_ > 0) {
        --cursorIndex_;
    }
}

void InputContext::handleCursorRight() {
    int pageCount = static_cast<int>(currentPageCandidates().size());
    if (cursorIndex_ < pageCount - 1) {
        ++cursorIndex_;
    }
}

} // namespace core
