#pragma once

#include <QColor>
#include <QFont>

class ThemeManager {
public:
    ThemeManager();

    QColor backgroundColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    QColor highlightColor() const;

    QFont candidateFont() const;
};

