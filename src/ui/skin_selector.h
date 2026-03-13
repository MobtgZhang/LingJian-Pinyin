#pragma once

#include <QWidget>
#include <QVector>
#include <QRectF>
#include <QColor>
#include <QString>
#include <QPropertyAnimation>

class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

class SkinSelector : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal scrollOffset READ scrollOffset WRITE setScrollOffset)
public:
    explicit SkinSelector(QWidget *parent = nullptr);

    struct SkinPreview {
        QString id;
        QString name;
        QColor background;
        QColor border;
        QColor text;
        QColor highlight;
        QColor accent;
        bool isCustom = false;
    };

    void addBuiltinSkins();
    void addSkin(const SkinPreview &skin);
    void setCurrentSkin(const QString &id);

    qreal scrollOffset() const { return scrollOffset_; }
    void setScrollOffset(qreal offset);

    void popup(const QPoint &pos);

signals:
    void skinSelected(const QString &skinId);
    void loadCustomSkinClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void rebuildLayout();
    int hitTestCard(const QPoint &pos) const;
    void animateScroll(qreal target);
    qreal maxScrollOffset() const;

    QVector<SkinPreview> skins_;
    QString currentSkinId_;

    qreal scrollOffset_ = 0.0;
    qreal targetScrollOffset_ = 0.0;
    int hoveredCard_ = -1;

    bool dragging_ = false;
    QPoint dragStartPos_;
    qreal dragStartOffset_ = 0.0;

    QPropertyAnimation *scrollAnim_ = nullptr;

    QRectF loadButtonRect_;
    bool loadButtonHovered_ = false;

    static constexpr int kCardWidth = 140;
    static constexpr int kCardHeight = 100;
    static constexpr int kCardSpacing = 12;
    static constexpr int kPanelPadding = 16;
    static constexpr int kPanelHeight = 180;
    static constexpr int kTitleHeight = 36;
    static constexpr int kRadius = 12;
    static constexpr int kShadowMargin = 8;
};
