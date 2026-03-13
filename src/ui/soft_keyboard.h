#pragma once

#include <QWidget>
#include <QVector>
#include <QRectF>
#include <QString>
#include <QColor>
#include <QPoint>

class SoftKeyboard : public QWidget {
    Q_OBJECT
public:
    explicit SoftKeyboard(QWidget *parent = nullptr);

    enum KeyboardType {
        PC_Keyboard = 0,
        GreekLetters,
        RussianLetters,
        ZhuyinSymbols,
        PinyinLetters,
        JapaneseHiragana,
        JapaneseKatakana,
        Punctuation,
        NumberSequence,
        MathSymbols,
        TableDrawing,
        ChineseNumbers,
        SpecialSymbols,
        KeyboardTypeCount
    };

    void setKeyboardType(KeyboardType type);
    KeyboardType keyboardType() const { return currentType_; }

    void popup(const QPoint &pos);

    void applySkinColors(const QColor &bg, const QColor &border,
                         const QColor &keyBg, const QColor &keyBorder,
                         const QColor &keyText, const QColor &keyHover,
                         const QColor &keyPressed,
                         const QColor &funcKeyBg,
                         const QColor &titleBg, const QColor &titleText,
                         int borderRadius);

signals:
    void keyPressed(const QString &text);
    void specialKeyPressed(Qt::Key key);
    void closed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    enum KeyRole {
        CharKey,
        FuncKey
    };

    struct KeyInfo {
        QString label;
        QString normalChar;
        QString shiftChar;
        double widthUnits;
        KeyRole role;
        Qt::Key qtKey;
        QRectF rect;
        int charIndex = -1;
    };

    void buildLayout();
    void remapKeys();
    int hitTestKey(const QPoint &pos) const;
    void showTypeMenu(const QPoint &globalPos);
    void drawKey(QPainter &p, int flatIdx, const KeyInfo &key) const;

    static QString typeDisplayName(KeyboardType type);
    static QString typePreview(KeyboardType type);
    static QVector<QString> characterSet(KeyboardType type);

    KeyboardType currentType_ = PC_Keyboard;
    bool capsLock_ = false;
    bool shiftHeld_ = false;

    QVector<QVector<KeyInfo>> rows_;
    int hoveredKey_ = -1;
    int pressedKey_ = -1;

    QRectF titleBarRect_;
    QRectF closeButtonRect_;
    bool closeHovered_ = false;
    bool titleDragging_ = false;
    QPoint dragOffset_;

    QColor bgColor_{230, 232, 234};
    QColor borderColor_{200, 200, 200};
    QColor keyBgColor_{255, 255, 255};
    QColor keyBorderColor_{185, 185, 185};
    QColor keyTextColor_{30, 30, 30};
    QColor keyHoverColor_{220, 230, 245};
    QColor keyPressedColor_{200, 210, 230};
    QColor funcKeyBgColor_{200, 202, 205};
    QColor titleBgColor_{66, 133, 244};
    QColor titleTextColor_{255, 255, 255};
    int borderRadius_ = 4;

    static constexpr int kTitleBarHeight = 30;
    static constexpr int kKeyHeight = 38;
    static constexpr int kKeyGap = 3;
    static constexpr int kPadding = 6;
    static constexpr double kUnitCell = 47.0;

    int flatIndex(int row, int col) const;
    QPair<int, int> rowColFromFlat(int flat) const;
    int totalFlatKeys() const;
};
