#include "more_input_submenu.h"
#include "theme_manager.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>

static constexpr int kMenuWidth = 160;
static constexpr int kItemHeight = 44;
static constexpr int kRadius = 12;
static constexpr int kHPadding = 16;
static constexpr int kShadowMargin = 8;

struct SubmenuItem {
    QString icon;
    QString text;
};

static const QVector<SubmenuItem> kItems = {
    {QStringLiteral("\U0001F399"), QStringLiteral("\u8bed\u97f3\u8f93\u5165")},
    {QStringLiteral("\u270D"),     QStringLiteral("\u624b\u5199\u8f93\u5165")},
    {QStringLiteral("\U0001F3A8"), QStringLiteral("\u76ae\u80a4\u5546\u57ce")},
    {QStringLiteral("\u2295"),     QStringLiteral("\u5b9a\u5236\u72b6\u6001\u680f")},
};

MoreInputSubmenu::MoreInputSubmenu(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setFixedSize(kMenuWidth + 2 * kShadowMargin,
                 static_cast<int>(kItems.size()) * kItemHeight + 2 * kShadowMargin);
}

void MoreInputSubmenu::popup(const QPoint &pos) {
    QScreen *screen = QApplication::screenAt(pos);
    if (!screen) screen = QApplication::primaryScreen();
    QRect sr = screen->availableGeometry();
    int x = pos.x();
    int y = pos.y();
    if (x + width() > sr.right() + 1)
        x = sr.right() + 1 - width();
    if (y + height() > sr.bottom() + 1)
        y = pos.y() - height();
    if (x < sr.left()) x = sr.left();
    if (y < sr.top()) y = sr.top();
    move(x, y);
    show();
    raise();
    activateWindow();
}

QRectF MoreInputSubmenu::itemRect(int index) const {
    if (index < 0 || index >= kItems.size()) return QRectF();
    return QRectF(kShadowMargin, kShadowMargin + index * kItemHeight, kMenuWidth, kItemHeight);
}

int MoreInputSubmenu::itemHitTest(const QPoint &pos) const {
    for (int i = 0; i < kItems.size(); ++i) {
        if (itemRect(i).contains(pos)) return i;
    }
    return -1;
}

void MoreInputSubmenu::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const auto &skin = ThemeManager::instance().skin();
    QRectF contentRect(kShadowMargin, kShadowMargin, kMenuWidth,
                      static_cast<qreal>(kItems.size()) * kItemHeight);

    // 阴影
    struct ShadowLayer { qreal spread; int alpha; };
    ShadowLayer layers[] = {
        {7.0, 3}, {5.0, 6}, {3.5, 10}, {2.0, 16}, {0.5, 22},
    };
    for (const auto &l : layers) {
        QPainterPath sp;
        sp.addRoundedRect(contentRect.adjusted(-l.spread, -l.spread, l.spread, l.spread),
                          kRadius + l.spread, kRadius + l.spread);
        p.fillPath(sp, QColor(0, 0, 0, l.alpha));
    }

    // 背景
    QPainterPath bgPath;
    bgPath.addRoundedRect(contentRect, kRadius, kRadius);
    p.fillPath(bgPath, skin.cvBackground);
    p.setPen(QPen(skin.cvBorder, 0.5));
    p.drawPath(bgPath);

    p.setClipPath(bgPath);

    const bool isDark = skin.cvBackground.lightness() < 128;
    const QColor hoverOverlay = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 35);

    QFont iconFont;
    iconFont.setPointSize(15);
    QFont textFont;
    textFont.setPointSize(12);
    textFont.setWeight(QFont::Normal);

    for (int i = 0; i < kItems.size(); ++i) {
        const auto &item = kItems[i];
        QRectF r = itemRect(i);

        if (hoveredItem_ == i) {
            QPainterPath hp;
            hp.addRoundedRect(r.adjusted(2, 1, -2, -1), 6, 6);
            p.fillPath(hp, hoverOverlay);
        }

        p.setFont(iconFont);
        p.setPen(skin.cvText);
        QRectF iconRect(r.left() + kHPadding, r.top(), 32, r.height());
        p.drawText(iconRect, Qt::AlignCenter, item.icon);

        p.setFont(textFont);
        p.setPen(skin.cvText);
        QRectF textRect(r.left() + kHPadding + 38, r.top(),
                        r.width() - kHPadding * 2, r.height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, item.text);
    }
}

void MoreInputSubmenu::mouseMoveEvent(QMouseEvent *event) {
    int idx = itemHitTest(event->pos());
    if (idx != hoveredItem_) {
        hoveredItem_ = idx;
        setCursor(hoveredItem_ >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void MoreInputSubmenu::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int idx = itemHitTest(event->pos());
        if (idx >= 0) {
            switch (idx) {
            case 0: emit voiceInputClicked(); break;
            case 1: emit handwritingInputClicked(); break;
            case 2: emit skinStoreClicked(); break;
            case 3: emit customizeStatusBarClicked(); break;
            default: break;
            }
            close();
            return;
        }
    }
    close();
}

void MoreInputSubmenu::leaveEvent(QEvent *) {
    hoveredItem_ = -1;
    unsetCursor();
    update();
}
