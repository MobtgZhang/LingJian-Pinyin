#include "context.h"

#include "decoder.h"

namespace core {

InputContext::InputContext()
    : decoder_(std::make_unique<Decoder>()) {
}

InputContext::~InputContext() = default;

void InputContext::handleKeyEvent(int keySym, int /*keyState*/) {
    // 这里仅做极简示例：假设 keySym 是 ASCII 字符
    if (keySym >= 32 && keySym < 127) {
        composingPinyin_.push_back(static_cast<char>(keySym));
    }

    currentCandidates_ = decoder_->decode(composingPinyin_);
}

std::string InputContext::preeditText() const {
    return composingPinyin_;
}

std::vector<CoreCandidate> InputContext::candidates() const {
    return currentCandidates_;
}

void InputContext::commitCandidate(std::size_t index) {
    if (index >= currentCandidates_.size()) {
        return;
    }
    // 真实实现中此处应通知上层框架上屏文本，并更新用户词典等。
    composingPinyin_.clear();
    currentCandidates_.clear();
}

} // namespace core

