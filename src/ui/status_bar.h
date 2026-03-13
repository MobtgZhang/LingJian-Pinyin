#pragma once

#include <QFrame>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QRectF>

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

    void setInputMode(InputMode mode);
    InputMode inputMode() const { return inputMode_; }

    void setPunctuationMode(PunctuationMode mode);
    PunctuationMode punctuationMode() const { return punctuationMode_; }

    enum class Skin { Dark, Light, Blue };
    void setSkin(Skin skin);

signals:
    void inputModeToggled(InputMode mode);
    void punctuationModeToggled(PunctuationMode mode);
    void voiceInputClicked();
    void keyboardClicked();
    void skinClicked();
    void aiClicked();

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
    Skin skin_ = Skin::Light;

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
