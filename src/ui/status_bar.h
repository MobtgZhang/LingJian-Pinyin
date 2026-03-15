#pragma once

#include <QFrame>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QRectF>
#include <QColor>

class QMouseEvent;
class QPaintEvent;
class QContextMenuEvent;
class StatusBarMenu;

class StatusBar : public QFrame {
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = nullptr);

    enum class InputMode { Chinese, English };
    enum class PunctuationMode { Chinese, English };
    enum class SimplifiedTraditional { Simplified, Traditional };
    enum class HalfFullWidth { Half, Full };

    void setInputMode(InputMode mode);
    InputMode inputMode() const { return inputMode_; }

    void setPunctuationMode(PunctuationMode mode);
    PunctuationMode punctuationMode() const { return punctuationMode_; }

    void setSimplifiedTraditional(SimplifiedTraditional mode);
    SimplifiedTraditional simplifiedTraditional() const { return simplifiedTraditional_; }

    void setHalfFullWidth(HalfFullWidth mode);
    HalfFullWidth halfFullWidth() const { return halfFullWidth_; }

    void applySkinColors(const QColor &bg, const QColor &border,
                         const QColor &text, const QColor &hover,
                         const QColor &logo, const QColor &ai,
                         int borderRadius);

    void showContextMenuAt(const QPoint &globalPos);

    /** 请求隐藏状态栏（与右键菜单「隐藏状态栏」行为一致） */
    void requestHide();

signals:
    void inputModeToggled(InputMode mode);
    void punctuationModeToggled(PunctuationMode mode);
    void simplifiedTraditionalToggled(SimplifiedTraditional mode);
    void halfFullWidthToggled(HalfFullWidth mode);
    void voiceInputClicked();
    void handwritingInputClicked();
    void keyboardClicked();
    void customizeStatusBarClicked();
    void skinClicked();
    void aiClicked();
    void hideRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    struct ButtonItem {
        QString label;
        QRectF rect;
        bool hovered = false;
    };

    void rebuildButtons();
    int hitTest(const QPoint &pos) const;

    InputMode inputMode_ = InputMode::Chinese;
    PunctuationMode punctuationMode_ = PunctuationMode::Chinese;
    SimplifiedTraditional simplifiedTraditional_ = SimplifiedTraditional::Simplified;
    HalfFullWidth halfFullWidth_ = HalfFullWidth::Full;

    QColor bgColor_{255, 255, 255, 245};
    QColor borderColor_{210, 210, 210};
    QColor textColor_{50, 50, 50};
    QColor hoverColor_{0, 0, 0, 18};
    QColor logoColor_{255, 120, 0};
    QColor aiColor_{40, 130, 220};
    int borderRadius_ = 6;

    QVector<ButtonItem> buttons_;
    bool dragging_ = false;
    bool mousePressed_ = false;
    QPoint dragPosition_;
    QPoint pressGlobalPos_;

    StatusBarMenu *contextMenu_ = nullptr;
    void ensureContextMenu();

    static constexpr int kButtonSize = 32;
    static constexpr int kBarHeight = 36;
    static constexpr int kPadding = 4;
    static constexpr int kSeparatorWidth = 1;
};
