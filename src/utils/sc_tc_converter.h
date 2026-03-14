#pragma once

#include <string>

namespace lingjian {

/** 简繁转换器：简体 ↔ 繁体 */
class ScTcConverter {
public:
    ScTcConverter();
    ~ScTcConverter();

    /** 简体转繁体 */
    std::string toTraditional(const std::string &utf8Simplified) const;

    /** 繁体转简体 */
    std::string toSimplified(const std::string &utf8Traditional) const;

    /** 是否可用（如 OpenCC 已加载） */
    bool isAvailable() const { return available_; }

private:
    bool available_ = false;
    void *s2t_ = nullptr;  // OpenCC 实例，不暴露头文件
    void *t2s_ = nullptr;
};

} // namespace lingjian
