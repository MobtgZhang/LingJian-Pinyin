#pragma once

#include <string>

namespace lingjian {

/** 简繁转换器：基于内置映射表实现简体 ↔ 繁体转换 */
class ScTcConverter {
public:
    ScTcConverter();
    ~ScTcConverter();

    std::string toTraditional(const std::string &utf8Simplified) const;
    std::string toSimplified(const std::string &utf8Traditional) const;

    bool isAvailable() const { return available_; }

private:
    bool available_ = false;
};

} // namespace lingjian
