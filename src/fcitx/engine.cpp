#include "engine.h"

#include "../core/context.h"

namespace fcitx_adapter {

Engine::Engine()
    : context_(std::make_unique<core::InputContext>()) {
}

Engine::~Engine() = default;

void Engine::handleKeyEvent(int keySym, int /*keyState*/) {
    if (!context_) {
        return;
    }
    if (keySym >= 32 && keySym < 127) {
        context_->handleKey(static_cast<char>(keySym));
    }
}

std::string Engine::preeditText() const {
    if (!context_) {
        return {};
    }
    return context_->preeditText();
}

std::vector<Candidate> Engine::candidates() const {
    std::vector<Candidate> result;
    if (!context_) {
        return result;
    }
    auto coreCandidates = context_->candidates();
    result.reserve(coreCandidates.size());
    for (const auto &c : coreCandidates) {
        result.push_back({c.text, c.comment});
    }
    return result;
}

void Engine::commitCandidate(std::size_t index) {
    if (!context_) {
        return;
    }
    context_->selectCandidate(index);
}

} // namespace fcitx_adapter
