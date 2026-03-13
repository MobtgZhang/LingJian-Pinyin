#include "engine.h"

// 这里原本应该包含 Fcitx5 的头文件，并按照其 addon 开发规范
// 注册一个输入法引擎。为方便在没有 Fcitx5 开发环境的情况下编译，
// 当前文件只保留一个占位符函数，后续可以替换为真实实现。

namespace fcitx_adapter {

Engine *createEngine() {
    static Engine engine;
    return &engine;
}

} // namespace fcitx_adapter

