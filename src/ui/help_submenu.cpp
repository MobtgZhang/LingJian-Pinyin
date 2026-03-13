#include "help_submenu.h"
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

HelpSubmenu::HelpSubmenu(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setFixedSize(kMenuWidth + 2 * kShadowMargin, kItemHeight + 2 * kShadowMargin);
}

void HelpSubmenu::popup(const QPoint &pos) {
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

QRectF HelpSubmenu::itemRect() const {
    return QRectF(kShadowMargin, kShadowMargin, kMenuWidth, kItemHeight);
}

void HelpSubmenu::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const auto &skin = ThemeManager::instance().skin();
    QRectF contentRect(kShadowMargin, kShadowMargin, kMenuWidth, kItemHeight);

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

    // 菜单项
    const bool isDark = skin.cvBackground.lightness() < 128;
    const QColor hoverOverlay = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 35);

    QRectF r = itemRect();
    if (hovered_) {
        QPainterPath hp;
        hp.addRoundedRect(r.adjusted(2, 1, -2, -1), 6, 6);
        p.fillPath(hp, hoverOverlay);
    }

    QFont iconFont;
    iconFont.setPointSize(15);
    p.setFont(iconFont);
    p.setPen(skin.sbLogo);
    QRectF iconRect(r.left() + kHPadding, r.top(), 32, r.height());
    p.drawText(iconRect, Qt::AlignCenter, QStringLiteral("\u7075"));

    QFont textFont;
    textFont.setPointSize(12);
    textFont.setWeight(QFont::Normal);
    p.setFont(textFont);
    p.setPen(skin.cvText);
    QRectF textRect(r.left() + kHPadding + 38, r.top(),
                    r.width() - kHPadding * 2, r.height());
    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
               QStringLiteral("\u5173\u4e8e\u7075\u952e"));
}

void HelpSubmenu::mouseMoveEvent(QMouseEvent *event) {
    bool h = itemRect().contains(event->pos());
    if (h != hovered_) {
        hovered_ = h;
        setCursor(hovered_ ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void HelpSubmenu::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && itemRect().contains(event->pos())) {
        emit aboutClicked();
        close();
        return;
    }
    close();
}

void HelpSubmenu::leaveEvent(QEvent *) {
    hovered_ = false;
    unsetCursor();
    update();
}
