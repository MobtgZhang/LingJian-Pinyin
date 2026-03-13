#pragma once

#include <memory>
#include <string>
#include <vector>

namespace core {
class InputContext;
}

namespace fcitx_adapter {

struct Candidate {
    std::string text;
    std::string comment;
};

class Engine {
public:
    Engine();
    ~Engine();

    void handleKeyEvent(int keySym, int keyState);
    std::string preeditText() const;
    std::vector<Candidate> candidates() const;
    void commitCandidate(std::size_t index);

private:
    std::unique_ptr<core::InputContext> context_;
};

} // namespace fcitx_adapter

