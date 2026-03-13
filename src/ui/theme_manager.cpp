#include "theme_manager.h"

ThemeManager::ThemeManager() = default;

QColor ThemeManager::backgroundColor() const {
    return QColor(30, 30, 30, 220);
}

QColor ThemeManager::borderColor() const {
    return QColor(80, 80, 80);
}

QColor ThemeManager::textColor() const {
    return QColor(240, 240, 240);
}

QColor ThemeManager::highlightColor() const {
    return QColor(0, 180, 255);
}

QFont ThemeManager::candidateFont() const {
    QFont font;
    font.setPointSize(12);
    return font;
}
