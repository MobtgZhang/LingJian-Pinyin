#include "soft_keyboard.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QApplication>
#include <QScreen>
#include <QMenu>
#include <QtMath>

SoftKeyboard::SoftKeyboard(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    buildLayout();
}

void SoftKeyboard::buildLayout() {
    rows_.clear();

    auto C = [](const QString &label, const QString &normal, const QString &shift,
                double w, int charIdx = -1) -> KeyInfo {
        return {label, normal, shift, w, CharKey, Qt::Key_unknown, {}, charIdx};
    };
    auto F = [](const QString &label, double w, Qt::Key k) -> KeyInfo {
        return {label, {}, {}, w, FuncKey, k, {}, -1};
    };

    int ci = 0;

    // Row 0: number row
    QVector<KeyInfo> row0;
    row0 << C(QStringLiteral("`"), QStringLiteral("`"), QStringLiteral("~"), 1.0, ci++);
    row0 << C(QStringLiteral("1"), QStringLiteral("1"), QStringLiteral("!"), 1.0, ci++);
    row0 << C(QStringLiteral("2"), QStringLiteral("2"), QStringLiteral("@"), 1.0, ci++);
    row0 << C(QStringLiteral("3"), QStringLiteral("3"), QStringLiteral("#"), 1.0, ci++);
    row0 << C(QStringLiteral("4"), QStringLiteral("4"), QStringLiteral("$"), 1.0, ci++);
    row0 << C(QStringLiteral("5"), QStringLiteral("5"), QStringLiteral("%"), 1.0, ci++);
    row0 << C(QStringLiteral("6"), QStringLiteral("6"), QStringLiteral("^"), 1.0, ci++);
    row0 << C(QStringLiteral("7"), QStringLiteral("7"), QStringLiteral("&"), 1.0, ci++);
    row0 << C(QStringLiteral("8"), QStringLiteral("8"), QStringLiteral("*"), 1.0, ci++);
    row0 << C(QStringLiteral("9"), QStringLiteral("9"), QStringLiteral("("), 1.0, ci++);
    row0 << C(QStringLiteral("0"), QStringLiteral("0"), QStringLiteral(")"), 1.0, ci++);
    row0 << C(QStringLiteral("-"), QStringLiteral("-"), QStringLiteral("_"), 1.0, ci++);
    row0 << C(QStringLiteral("="), QStringLiteral("="), QStringLiteral("+"), 1.0, ci++);
    row0 << F(QStringLiteral("\u2190"), 1.5, Qt::Key_Backspace);

    // Row 1: QWERTY top
    QVector<KeyInfo> row1;
    row1 << F(QStringLiteral("Tab"), 1.5, Qt::Key_Tab);
    row1 << C(QStringLiteral("q"), QStringLiteral("q"), QStringLiteral("Q"), 1.0, ci++);
    row1 << C(QStringLiteral("w"), QStringLiteral("w"), QStringLiteral("W"), 1.0, ci++);
    row1 << C(QStringLiteral("e"), QStringLiteral("e"), QStringLiteral("E"), 1.0, ci++);
    row1 << C(QStringLiteral("r"), QStringLiteral("r"), QStringLiteral("R"), 1.0, ci++);
    row1 << C(QStringLiteral("t"), QStringLiteral("t"), QStringLiteral("T"), 1.0, ci++);
    row1 << C(QStringLiteral("y"), QStringLiteral("y"), QStringLiteral("Y"), 1.0, ci++);
    row1 << C(QStringLiteral("u"), QStringLiteral("u"), QStringLiteral("U"), 1.0, ci++);
    row1 << C(QStringLiteral("i"), QStringLiteral("i"), QStringLiteral("I"), 1.0, ci++);
    row1 << C(QStringLiteral("o"), QStringLiteral("o"), QStringLiteral("O"), 1.0, ci++);
    row1 << C(QStringLiteral("p"), QStringLiteral("p"), QStringLiteral("P"), 1.0, ci++);
    row1 << C(QStringLiteral("["), QStringLiteral("["), QStringLiteral("{"), 1.0, ci++);
    row1 << C(QStringLiteral("]"), QStringLiteral("]"), QStringLiteral("}"), 1.0, ci++);
    row1 << C(QStringLiteral("\\"), QStringLiteral("\\"), QStringLiteral("|"), 1.0, ci++);

    // Row 2: home row
    QVector<KeyInfo> row2;
    row2 << F(QStringLiteral("Caps"), 1.75, Qt::Key_CapsLock);
    row2 << C(QStringLiteral("a"), QStringLiteral("a"), QStringLiteral("A"), 1.0, ci++);
    row2 << C(QStringLiteral("s"), QStringLiteral("s"), QStringLiteral("S"), 1.0, ci++);
    row2 << C(QStringLiteral("d"), QStringLiteral("d"), QStringLiteral("D"), 1.0, ci++);
    row2 << C(QStringLiteral("f"), QStringLiteral("f"), QStringLiteral("F"), 1.0, ci++);
    row2 << C(QStringLiteral("g"), QStringLiteral("g"), QStringLiteral("G"), 1.0, ci++);
    row2 << C(QStringLiteral("h"), QStringLiteral("h"), QStringLiteral("H"), 1.0, ci++);
    row2 << C(QStringLiteral("j"), QStringLiteral("j"), QStringLiteral("J"), 1.0, ci++);
    row2 << C(QStringLiteral("k"), QStringLiteral("k"), QStringLiteral("K"), 1.0, ci++);
    row2 << C(QStringLiteral("l"), QStringLiteral("l"), QStringLiteral("L"), 1.0, ci++);
    row2 << C(QStringLiteral(";"), QStringLiteral(";"), QStringLiteral(":"), 1.0, ci++);
    row2 << C(QStringLiteral("'"), QStringLiteral("'"), QStringLiteral("\""), 1.0, ci++);
    row2 << F(QStringLiteral("Enter"), 1.75, Qt::Key_Return);

    // Row 3: bottom letter row
    QVector<KeyInfo> row3;
    row3 << F(QStringLiteral("\u2191 Shift"), 2.25, Qt::Key_Shift);
    row3 << C(QStringLiteral("z"), QStringLiteral("z"), QStringLiteral("Z"), 1.0, ci++);
    row3 << C(QStringLiteral("x"), QStringLiteral("x"), QStringLiteral("X"), 1.0, ci++);
    row3 << C(QStringLiteral("c"), QStringLiteral("c"), QStringLiteral("C"), 1.0, ci++);
    row3 << C(QStringLiteral("v"), QStringLiteral("v"), QStringLiteral("V"), 1.0, ci++);
    row3 << C(QStringLiteral("b"), QStringLiteral("b"), QStringLiteral("B"), 1.0, ci++);
    row3 << C(QStringLiteral("n"), QStringLiteral("n"), QStringLiteral("N"), 1.0, ci++);
    row3 << C(QStringLiteral("m"), QStringLiteral("m"), QStringLiteral("M"), 1.0, ci++);
    row3 << C(QStringLiteral(","), QStringLiteral(","), QStringLiteral("<"), 1.0, ci++);
    row3 << C(QStringLiteral("."), QStringLiteral("."), QStringLiteral(">"), 1.0, ci++);
    row3 << C(QStringLiteral("/"), QStringLiteral("/"), QStringLiteral("?"), 1.0, ci++);

    // Row 4: space bar row
    QVector<KeyInfo> row4;
    row4 << F(QStringLiteral("Ins"), 1.5, Qt::Key_Insert);
    row4 << F(QStringLiteral("Del"), 1.5, Qt::Key_Delete);
    row4 << C(QString(), QStringLiteral(" "), QStringLiteral(" "), 10.0, -1);
    row4 << F(QStringLiteral("Esc"), 1.5, Qt::Key_Escape);

    rows_ = {row0, row1, row2, row3, row4};

    // Compute pixel rects
    double maxRowWidth = 0;
    for (const auto &row : rows_) {
        double total = 0;
        for (const auto &k : row)
            total += k.widthUnits;
        double rowPx = total * kUnitCell - kKeyGap;
        if (rowPx > maxRowWidth) maxRowWidth = rowPx;
    }

    double totalWidth = maxRowWidth + 2 * kPadding;
    double totalHeight = kTitleBarHeight + kPadding
                         + rows_.size() * kKeyHeight
                         + (rows_.size() - 1) * kKeyGap
                         + kPadding;

    titleBarRect_ = QRectF(0, 0, totalWidth, kTitleBarHeight);
    closeButtonRect_ = QRectF(totalWidth - kTitleBarHeight, 0,
                              kTitleBarHeight, kTitleBarHeight);

    double y = kTitleBarHeight + kPadding;
    for (auto &row : rows_) {
        double x = kPadding;
        for (auto &key : row) {
            double keyW = key.widthUnits * kUnitCell - kKeyGap;
            key.rect = QRectF(x, y, keyW, kKeyHeight);
            x += key.widthUnits * kUnitCell;
        }
        y += kKeyHeight + kKeyGap;
    }

    setFixedSize(qCeil(totalWidth), qCeil(totalHeight));
}

void SoftKeyboard::remapKeys() {
    if (currentType_ == PC_Keyboard) {
        buildLayout();
        return;
    }

    QVector<QString> chars = characterSet(currentType_);
    int ci = 0;
    for (auto &row : rows_) {
        for (auto &key : row) {
            if (key.role != CharKey || key.charIndex < 0)
                continue;
            if (ci < chars.size()) {
                key.label = chars[ci];
                key.normalChar = chars[ci];
                key.shiftChar = chars[ci];
            } else {
                key.label = QString();
                key.normalChar = QString();
                key.shiftChar = QString();
            }
            ++ci;
        }
    }
    update();
}

void SoftKeyboard::setKeyboardType(KeyboardType type) {
    if (currentType_ == type) return;
    currentType_ = type;
    buildLayout();
    if (type != PC_Keyboard)
        remapKeys();
    update();
}

void SoftKeyboard::popup(const QPoint &pos) {
    QScreen *screen = QApplication::screenAt(pos);
    if (!screen) screen = QApplication::primaryScreen();
    QRect sr = screen->availableGeometry();

    int x = pos.x() - width() / 2;
    int y = pos.y() - height();
    if (x + width() > sr.right()) x = sr.right() - width();
    if (x < sr.left()) x = sr.left();
    if (y < sr.top()) y = pos.y();
    if (y + height() > sr.bottom()) y = sr.bottom() - height();

    move(x, y);
    show();
    raise();
    activateWindow();
}

void SoftKeyboard::applySkinColors(const QColor &bg, const QColor &border,
                                    const QColor &keyBg, const QColor &keyBorder,
                                    const QColor &keyText, const QColor &keyHover,
                                    const QColor &keyPressed,
                                    const QColor &funcKeyBg,
                                    const QColor &titleBg, const QColor &titleText,
                                    int borderRadius)
{
    bgColor_ = bg;
    borderColor_ = border;
    keyBgColor_ = keyBg;
    keyBorderColor_ = keyBorder;
    keyTextColor_ = keyText;
    keyHoverColor_ = keyHover;
    keyPressedColor_ = keyPressed;
    funcKeyBgColor_ = funcKeyBg;
    titleBgColor_ = titleBg;
    titleTextColor_ = titleText;
    borderRadius_ = borderRadius;
    update();
}

int SoftKeyboard::flatIndex(int row, int col) const {
    int idx = 0;
    for (int r = 0; r < row && r < rows_.size(); ++r)
        idx += rows_[r].size();
    return idx + col;
}

QPair<int, int> SoftKeyboard::rowColFromFlat(int flat) const {
    int idx = 0;
    for (int r = 0; r < rows_.size(); ++r) {
        if (flat < idx + rows_[r].size())
            return {r, flat - idx};
        idx += rows_[r].size();
    }
    return {-1, -1};
}

int SoftKeyboard::totalFlatKeys() const {
    int n = 0;
    for (const auto &row : rows_) n += row.size();
    return n;
}

int SoftKeyboard::hitTestKey(const QPoint &pos) const {
    int idx = 0;
    for (const auto &row : rows_) {
        for (const auto &key : row) {
            if (key.rect.contains(pos))
                return idx;
            ++idx;
        }
    }
    return -1;
}

void SoftKeyboard::drawKey(QPainter &p, int flatIdx, const KeyInfo &key) const {
    if (key.rect.isEmpty()) return;

    bool hovered = (flatIdx == hoveredKey_);
    bool pressed = (flatIdx == pressedKey_);
    bool isFunc = (key.role == FuncKey);
    bool isCapsActive = (key.qtKey == Qt::Key_CapsLock && capsLock_);
    bool isShiftActive = (key.qtKey == Qt::Key_Shift && shiftHeld_);

    QColor face;
    if (pressed)
        face = keyPressedColor_;
    else if (hovered)
        face = keyHoverColor_;
    else if (isFunc)
        face = funcKeyBgColor_;
    else
        face = keyBgColor_;

    if (isCapsActive || isShiftActive) {
        face = keyPressedColor_;
    }

    QRectF r = key.rect;
    QPainterPath kp;
    kp.addRoundedRect(r, borderRadius_, borderRadius_);

    // Key shadow (subtle bottom edge)
    QColor shadowColor = keyBorderColor_;
    shadowColor.setAlpha(qMin(shadowColor.alpha(), 120));
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(r.adjusted(0, 1, 0, 1), borderRadius_, borderRadius_);
    p.fillPath(shadowPath, shadowColor);

    p.fillPath(kp, face);
    p.setPen(QPen(keyBorderColor_, 0.8));
    p.drawPath(kp);

    // Label
    QFont font;
    if (isFunc) {
        font.setPointSize(key.label.size() > 3 ? 10 : 11);
        font.setWeight(QFont::Medium);
    } else {
        font.setPointSize(13);
        font.setWeight(QFont::Normal);
    }

    // For space bar, show "Space" label
    QString displayLabel = key.label;
    if (key.normalChar == QStringLiteral(" ") && displayLabel.isEmpty())
        displayLabel = QString();

    bool shifted = capsLock_ ^ shiftHeld_;
    if (key.role == CharKey && currentType_ == PC_Keyboard && !key.normalChar.isEmpty()) {
        if (key.normalChar.at(0).isLetter() && shifted) {
            displayLabel = key.shiftChar;
        }
    }

    p.setFont(font);
    p.setPen(keyTextColor_);
    p.drawText(r, Qt::AlignCenter, displayLabel);

    // For number/symbol keys in PC mode, show shift char in top-right corner
    if (currentType_ == PC_Keyboard && key.role == CharKey
        && !key.shiftChar.isEmpty() && !key.normalChar.isEmpty()
        && !key.normalChar.at(0).isLetter() && key.shiftChar != key.normalChar) {
        QFont smallFont;
        smallFont.setPointSize(8);
        p.setFont(smallFont);
        QColor dimColor = keyTextColor_;
        dimColor.setAlpha(140);
        p.setPen(dimColor);
        QRectF topRight(r.left() + 2, r.top() + 2, r.width() - 4, r.height() / 2 - 2);
        p.drawText(topRight, Qt::AlignRight | Qt::AlignTop, key.shiftChar);
    }
}

void SoftKeyboard::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF bgRect(0, kTitleBarHeight, width(), height() - kTitleBarHeight);
    QPainterPath bgPath;
    bgPath.addRoundedRect(bgRect.adjusted(0, 0, 0, 0), 0, 0);

    // Full background with rounded bottom corners
    QPainterPath fullBg;
    fullBg.addRoundedRect(QRectF(0, 0, width(), height()), borderRadius_ + 2, borderRadius_ + 2);
    p.fillPath(fullBg, bgColor_);
    p.setPen(QPen(borderColor_, 1));
    p.drawPath(fullBg);

    // Title bar
    QPainterPath titlePath;
    titlePath.moveTo(borderRadius_ + 2, 0);
    titlePath.arcTo(QRectF(0, 0, (borderRadius_ + 2) * 2, (borderRadius_ + 2) * 2),
                    90, 90);
    titlePath.lineTo(0, kTitleBarHeight);
    titlePath.lineTo(width(), kTitleBarHeight);
    titlePath.lineTo(width(), borderRadius_ + 2);
    titlePath.arcTo(QRectF(width() - (borderRadius_ + 2) * 2, 0,
                           (borderRadius_ + 2) * 2, (borderRadius_ + 2) * 2),
                    0, 90);
    titlePath.closeSubpath();
    p.fillPath(titlePath, titleBgColor_);

    // Title text
    QFont titleFont;
    titleFont.setPointSize(11);
    titleFont.setWeight(QFont::Medium);
    p.setFont(titleFont);
    p.setPen(titleTextColor_);

    QString title = QStringLiteral("\u7075\u952e\u8f6f\u952e\u76d8");
    if (currentType_ != PC_Keyboard) {
        title += QStringLiteral(" - ") + typeDisplayName(currentType_);
    }
    QRectF titleTextRect(12, 0, width() - kTitleBarHeight - 16, kTitleBarHeight);
    p.drawText(titleTextRect, Qt::AlignVCenter | Qt::AlignLeft, title);

    // Close button
    QColor closeBtnColor = titleTextColor_;
    if (closeHovered_) {
        QPainterPath closeHoverPath;
        closeHoverPath.addRoundedRect(closeButtonRect_.adjusted(4, 4, -4, -4), 3, 3);
        p.fillPath(closeHoverPath, QColor(255, 255, 255, 50));
    }
    QFont closeFont;
    closeFont.setPointSize(14);
    closeFont.setWeight(QFont::Normal);
    p.setFont(closeFont);
    p.setPen(closeBtnColor);
    p.drawText(closeButtonRect_, Qt::AlignCenter, QStringLiteral("\u00D7"));

    // Draw keys
    int flatIdx = 0;
    for (const auto &row : rows_) {
        for (const auto &key : row) {
            drawKey(p, flatIdx, key);
            ++flatIdx;
        }
    }
}

void SoftKeyboard::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    QPoint pos = event->pos();

    if (closeButtonRect_.contains(pos)) {
        hide();
        emit closed();
        return;
    }

    if (titleBarRect_.contains(pos) && !closeButtonRect_.contains(pos)) {
        titleDragging_ = true;
        dragOffset_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
        setCursor(Qt::SizeAllCursor);
        return;
    }

    int idx = hitTestKey(pos);
    if (idx >= 0) {
        pressedKey_ = idx;
        update();
    }
}

void SoftKeyboard::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    if (titleDragging_) {
        titleDragging_ = false;
        unsetCursor();
        return;
    }

    int idx = hitTestKey(event->pos());
    if (idx >= 0 && idx == pressedKey_) {
        auto [r, c] = rowColFromFlat(idx);
        if (r >= 0 && c >= 0) {
            const auto &key = rows_[r][c];
            if (key.role == FuncKey) {
                switch (key.qtKey) {
                case Qt::Key_CapsLock:
                    capsLock_ = !capsLock_;
                    break;
                case Qt::Key_Shift:
                    shiftHeld_ = !shiftHeld_;
                    break;
                case Qt::Key_Escape:
                    hide();
                    emit closed();
                    break;
                default:
                    emit specialKeyPressed(key.qtKey);
                    break;
                }
            } else {
                bool shifted = capsLock_ ^ shiftHeld_;
                QString ch;
                if (key.normalChar.isEmpty()) {
                    // empty key, do nothing
                } else if (key.normalChar.at(0).isLetter() && shifted) {
                    ch = key.shiftChar;
                } else if (!key.normalChar.at(0).isLetter() && shiftHeld_) {
                    ch = key.shiftChar;
                } else {
                    ch = key.normalChar;
                }
                if (!ch.isEmpty()) {
                    emit keyPressed(ch);
                    if (shiftHeld_) {
                        shiftHeld_ = false;
                    }
                }
            }
        }
    }

    pressedKey_ = -1;
    update();
}

void SoftKeyboard::mouseMoveEvent(QMouseEvent *event) {
    if (titleDragging_) {
        move(event->globalPosition().toPoint() - dragOffset_);
        return;
    }

    QPoint pos = event->pos();
    bool newCloseHover = closeButtonRect_.contains(pos);
    int newHovered = hitTestKey(pos);

    if (newCloseHover != closeHovered_ || newHovered != hoveredKey_) {
        closeHovered_ = newCloseHover;
        hoveredKey_ = newHovered;
        if (newCloseHover || newHovered >= 0)
            setCursor(Qt::PointingHandCursor);
        else
            unsetCursor();
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void SoftKeyboard::leaveEvent(QEvent *) {
    hoveredKey_ = -1;
    closeHovered_ = false;
    unsetCursor();
    update();
}

void SoftKeyboard::contextMenuEvent(QContextMenuEvent *event) {
    showTypeMenu(event->globalPos());
}

void SoftKeyboard::showTypeMenu(const QPoint &globalPos) {
    QMenu menu(this);
    menu.setStyleSheet(QStringLiteral(
        "QMenu {"
        "  background: #FAFBFC;"
        "  border: 1px solid #E1E4E8;"
        "  border-radius: 10px;"
        "  padding: 8px 4px;"
        "  min-width: 280px;"
        "}"
        "QMenu::item {"
        "  padding: 10px 20px;"
        "  font-size: 13px;"
        "  color: #1E1E1E;"
        "  border-radius: 6px;"
        "  margin: 2px 6px;"
        "}"
        "QMenu::item:selected {"
        "  background: #E8F0FE;"
        "  color: #1E1E1E;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background: #E8EAED;"
        "  margin: 6px 12px;"
        "}"));

    struct TypeEntry { KeyboardType type; QString shortcut; };
    QVector<TypeEntry> entries = {
        {PC_Keyboard,       QStringLiteral("1")},
        {GreekLetters,      QStringLiteral("2")},
        {RussianLetters,    QStringLiteral("3")},
        {ZhuyinSymbols,     QStringLiteral("4")},
        {PinyinLetters,     QStringLiteral("5")},
        {JapaneseHiragana,  QStringLiteral("6")},
        {JapaneseKatakana,  QStringLiteral("7")},
        {Punctuation,       QStringLiteral("8")},
        {NumberSequence,    QStringLiteral("9")},
        {MathSymbols,       QStringLiteral("0")},
        {TableDrawing,      QStringLiteral("A")},
        {ChineseNumbers,    QStringLiteral("B")},
        {SpecialSymbols,    QStringLiteral("C")},
    };

    for (const auto &e : entries) {
        QString text = e.shortcut + QStringLiteral("  ")
                     + typeDisplayName(e.type) + QStringLiteral("    ")
                     + typePreview(e.type);
        QAction *action = menu.addAction(text);
        action->setCheckable(true);
        action->setChecked(e.type == currentType_);
        connect(action, &QAction::triggered, this, [this, t = e.type]() {
            setKeyboardType(t);
        });
    }

    menu.addSeparator();

    QAction *closeAction = menu.addAction(
        QStringLiteral("\u5173\u95ed\u8f6f\u952e\u76d8 (L)"));
    connect(closeAction, &QAction::triggered, this, [this]() {
        hide();
        emit closed();
    });

    menu.exec(globalPos);
}

// ---- Static helpers for keyboard type data ----

QString SoftKeyboard::typeDisplayName(KeyboardType type) {
    switch (type) {
    case PC_Keyboard:       return QStringLiteral("PC \u952e\u76d8");
    case GreekLetters:      return QStringLiteral("\u5e0c\u814a\u5b57\u6bcd");
    case RussianLetters:    return QStringLiteral("\u4fc4\u6587\u5b57\u6bcd");
    case ZhuyinSymbols:     return QStringLiteral("\u6ce8\u97f3\u7b26\u53f7");
    case PinyinLetters:     return QStringLiteral("\u62fc\u97f3\u5b57\u6bcd");
    case JapaneseHiragana:  return QStringLiteral("\u65e5\u6587\u5e73\u5047\u540d");
    case JapaneseKatakana:  return QStringLiteral("\u65e5\u6587\u7247\u5047\u540d");
    case Punctuation:       return QStringLiteral("\u6807\u70b9\u7b26\u53f7");
    case NumberSequence:    return QStringLiteral("\u6570\u5b57\u5e8f\u53f7");
    case MathSymbols:       return QStringLiteral("\u6570\u5b66\u7b26\u53f7");
    case TableDrawing:      return QStringLiteral("\u5236\u8868\u7b26");
    case ChineseNumbers:    return QStringLiteral("\u4e2d\u6587\u6570\u5b57");
    case SpecialSymbols:    return QStringLiteral("\u7279\u6b8a\u7b26\u53f7");
    default:                return {};
    }
}

QString SoftKeyboard::typePreview(KeyboardType type) {
    switch (type) {
    case PC_Keyboard:       return QStringLiteral("asdfghjkl;");
    case GreekLetters:      return QStringLiteral("\u03b1\u03b2\u03b3\u03b4\u03b5");
    case RussianLetters:    return QStringLiteral("\u0430\u0431\u0432\u0433\u0434");
    case ZhuyinSymbols:     return QStringLiteral("\u3105\u3106\u3107\u3108");
    case PinyinLetters:     return QStringLiteral("\u0101\u00e1\u01ce\u00e8\u00f3");
    case JapaneseHiragana:  return QStringLiteral("\u3042\u3044\u3046\u3048\u304a");
    case JapaneseKatakana:  return QStringLiteral("\u30a2\u30a4\u30a6\u30f4\u30a7");
    case Punctuation:       return QStringLiteral("\u300e||\u3005\u300f");
    case NumberSequence:    return QStringLiteral("\u2160\u2161\u2162(\u4e00)\u2460");
    case MathSymbols:       return QStringLiteral("\u00b1\u00d7\u00f7\u2211\u221a");
    case TableDrawing:      return QStringLiteral("\u253c\u2524\u251c\u252c");
    case ChineseNumbers:    return QStringLiteral("\u58f9\u8d30\u5343\u4e07\u5146");
    case SpecialSymbols:    return QStringLiteral("\u25b2\u2606\u25c6\u25a1\u2192");
    default:                return {};
    }
}

QVector<QString> SoftKeyboard::characterSet(KeyboardType type) {
    QVector<QString> chars;
    auto fill = [&](const QString &s) {
        for (const QChar &ch : s) chars.append(QString(ch));
    };

    switch (type) {
    case GreekLetters:
        fill(QStringLiteral("\u03b1\u03b2\u03b3\u03b4\u03b5\u03b6\u03b7\u03b8"
                            "\u03b9\u03ba\u03bb\u03bc\u03bd\u03be\u03bf\u03c0"
                            "\u03c1\u03c3\u03c4\u03c5\u03c6\u03c7\u03c8\u03c9"
                            "\u0391\u0392\u0393\u0394\u0395\u0396\u0397\u0398"
                            "\u0399\u039a\u039b\u039c\u039d\u039e\u039f\u03a0"
                            "\u03a1\u03a3\u03a4\u03a5\u03a6\u03a7\u03a8\u03a9"));
        break;

    case RussianLetters:
        fill(QStringLiteral("\u0430\u0431\u0432\u0433\u0434\u0435\u0451\u0436"
                            "\u0437\u0438\u0439\u043a\u043b\u043c\u043d\u043e"
                            "\u043f\u0440\u0441\u0442\u0443\u0444\u0445\u0446"
                            "\u0447\u0448\u0449\u044a\u044b\u044c\u044d\u044e"
                            "\u044f\u0410\u0411\u0412\u0413\u0414\u0415\u0401"
                            "\u0416\u0417\u0418\u0419\u041a\u041b\u041c"));
        break;

    case ZhuyinSymbols:
        fill(QStringLiteral("\u3105\u3106\u3107\u3108\u3109\u310a\u310b\u310c"
                            "\u310d\u310e\u310f\u3110\u3111\u3112\u3113\u3114"
                            "\u3115\u3116\u3117\u3118\u3119\u311a\u311b\u311c"
                            "\u311d\u311e\u311f\u3120\u3121\u3122\u3123\u3124"
                            "\u3125\u3126\u3127\u3128\u3129"));
        break;

    case PinyinLetters:
        fill(QStringLiteral("\u0101\u00e1\u01ce\u00e0\u014d\u00f3\u01d2\u00f2"
                            "\u0113\u00e9\u011b\u00e8\u012b\u00ed\u01d0\u00ec"
                            "\u016b\u00fa\u01d4\u00f9\u01d6\u01d8\u01da\u01dc"
                            "\u0100\u00c1\u01cd\u00c0\u014c\u00d3\u01d1\u00d2"
                            "\u0112\u00c9\u011a\u00c8\u012a\u00cd\u01cf\u00cc"
                            "\u016a\u00da\u01d3\u00d9\u01d5\u01d7"));
        break;

    case JapaneseHiragana:
        fill(QStringLiteral("\u3042\u3044\u3046\u3048\u304a\u304b\u304d\u304f"
                            "\u3051\u3053\u3055\u3057\u3059\u305b\u305d\u305f"
                            "\u3061\u3064\u3066\u3068\u306a\u306b\u306c\u306d"
                            "\u306e\u306f\u3072\u3075\u3078\u307b\u307e\u307f"
                            "\u3080\u3081\u3082\u3084\u3086\u3088\u3089\u308a"
                            "\u308b\u308c\u308d\u308f\u3092\u3093"));
        break;

    case JapaneseKatakana:
        fill(QStringLiteral("\u30a2\u30a4\u30a6\u30a8\u30aa\u30ab\u30ad\u30af"
                            "\u30b1\u30b3\u30b5\u30b7\u30b9\u30bb\u30bd\u30bf"
                            "\u30c1\u30c4\u30c6\u30c8\u30ca\u30cb\u30cc\u30cd"
                            "\u30ce\u30cf\u30d2\u30d5\u30d8\u30db\u30de\u30df"
                            "\u30e0\u30e1\u30e2\u30e4\u30e6\u30e8\u30e9\u30ea"
                            "\u30eb\u30ec\u30ed\u30ef\u30f2\u30f3"));
        break;

    case Punctuation:
        fill(QStringLiteral("\u3001\u3002\u00b7\u02c9\u02c7\u00a8\u3003\u3005"
                            "\u2014\uff5e\u2016\u2026\u2018\u2019\u201c\u201d"
                            "\u3014\u3015\u3008\u3009\u300a\u300b\u300c\u300d"
                            "\u300e\u300f\u3010\u3011\uff01\uff1f\uff0c\uff0e"
                            "\uff1b\uff1a\uff08\uff09\u3016\u3017\uff5b\uff5d"
                            "\u3002\u3001\uff1a\uff1b\uff01"));
        break;

    case NumberSequence:
        fill(QStringLiteral("\u2160\u2161\u2162\u2163\u2164\u2165\u2166\u2167"
                            "\u2168\u2169\u216a\u216b\u2474\u2475\u2476\u2477"
                            "\u2478\u2479\u247a\u247b\u247c\u247d\u2460\u2461"
                            "\u2462\u2463\u2464\u2465\u2466\u2467\u2468\u2469"
                            "\u246a\u246b\u246c\u246d\u246e\u246f\u2470\u2471"
                            "\u2472\u2473\u2480\u2481\u2482"));
        break;

    case MathSymbols:
        fill(QStringLiteral("\u00b1\u00d7\u00f7\u2211\u221a\u220f\u222a\u2229"
                            "\u2208\u2209\u2205\u2282\u2283\u2286\u2287\u221e"
                            "\u221d\u2220\u2225\u22a5\u2248\u2261\u2260\u2264"
                            "\u2265\u003c\u003e\u2234\u2235\u2237\u222b\u222c"
                            "\u2202\u2207\u2312\u2299\u2295\u2297\u2296\u2218"
                            "\u2227\u2228\u00ac\u21d2\u21d4"));
        break;

    case TableDrawing:
        fill(QStringLiteral("\u250c\u2510\u2514\u2518\u251c\u2524\u252c\u2534"
                            "\u253c\u2500\u2502\u2554\u2557\u255a\u255d\u2560"
                            "\u2563\u2566\u2569\u256c\u2550\u2551\u2552\u2555"
                            "\u2558\u255b\u2553\u2556\u2559\u255c\u255e\u2561"
                            "\u2564\u2567\u256a\u255f\u2562\u2565\u2568\u256b"
                            "\u2574\u2575\u2576\u2577\u2578"));
        break;

    case ChineseNumbers:
        fill(QStringLiteral("\u96f6\u58f9\u8d30\u53c1\u8086\u4f0d\u9646\u67d2"
                            "\u634c\u7396\u62fe\u4f70\u4edf\u4e07\u4ebf\u5146"
                            "\u5409\u592a\u62cd\u827e\u5206\u5398\u6beb\u5fae"
                            "\u7eb3\u76ae\u98de\u963f\u6cd5\u5143\u89d2\u5206"
                            "\u5341\u767e\u5343\u4e07\u4ebf\u5146\u4eac\u5793"
                            "\u79ed\u7a70\u6c9f\u6da7\u6b63"));
        break;

    case SpecialSymbols:
        fill(QStringLiteral("\u2605\u2606\u25cb\u25cf\u25ce\u25c7\u25c6\u25a1"
                            "\u25a0\u25b3\u25b2\u25bd\u25bc\u2192\u2190\u2191"
                            "\u2193\u2197\u2198\u2199\u2196\u2642\u2640\u266a"
                            "\u266c\u00a7\u203b\u2020\u2021\u00b6\u2103\u2109"
                            "\u00a9\u00ae\u2122\u2030\u221e\u2234\u2235\u223c"
                            "\u2252\u226e\u226f\u2261\u2295"));
        break;

    default:
        break;
    }

    return chars;
}
