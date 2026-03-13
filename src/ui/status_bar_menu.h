#pragma once

#include <QWidget>
#include <QVector>
#include <QRectF>
#include <QString>

class StatusBarMenu : public QWidget {
    Q_OBJECT
public:
    explicit StatusBarMenu(QWidget *parent = nullptr);

    void popup(const QPoint &pos);

signals:
    void simplifiedTraditionalToggled();
    void halfFullWidthToggled();
    void chineseEnglishToggled();
    void hideStatusBarClicked();
    void voiceInputClicked();
    void symbolsClicked();
    void softKeyboardClicked();
    void emojiClicked();
    void moreInputClicked();
    void skinStoreClicked();
    void aiToolsClicked();
    void customizeStatusBarClicked();
    void helpClicked();
    void globalSettingsClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    struct ToggleItem {
        QString character;
        QString subtitle;
        QRectF rect;
    };

    struct MenuItem {
        QString icon;
        QString text;
        bool hasSubmenu;
        QRectF rect;
    };

    void buildLayout();
    int toggleHitTest(const QPoint &pos) const;
    int itemHitTest(const QPoint &pos) const;

    QVector<ToggleItem> toggles_;
    QVector<MenuItem> items_;
    QVector<qreal> separatorYs_;

    int hoveredToggle_ = -1;
    int hoveredItem_ = -1;
    int contentHeight_ = 0;

    static constexpr int kMenuWidth = 230;
    static constexpr int kTopSectionHeight = 88;
    static constexpr int kItemHeight = 44;
    static constexpr int kRadius = 12;
    static constexpr int kHPadding = 16;
    static constexpr int kShadowMargin = 8;
};
