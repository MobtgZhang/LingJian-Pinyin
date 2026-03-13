#include "status_bar_menu.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>

StatusBarMenu::StatusBarMenu(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    buildLayout();
}

void StatusBarMenu::buildLayout() {
    toggles_.clear();
    items_.clear();
    separatorYs_.clear();

    const qreal toggleWidth = kMenuWidth / 4.0;
    const qreal toggleTop = 10;
    const qreal toggleHeight = kTopSectionHeight - 20;

    toggles_ = {
        {QStringLiteral("\u7b80"), QStringLiteral("\u7b80\u7e41\u5207\u6362"), {}},
        {QStringLiteral("\u534a"), QStringLiteral("\u5168\u534a\u89d2\u5207\u6362"), {}},
        {QStringLiteral("\u82f1"), QStringLiteral("\u82f1\u6587\u8f93\u5165\u6cd5"), {}},
        {QStringLiteral("\U0001F441"), QStringLiteral("\u9690\u85cf\u72b6\u6001\u680f"), {}},
    };

    for (int i = 0; i < toggles_.size(); ++i) {
        toggles_[i].rect = QRectF(
            kShadowMargin + i * toggleWidth,
            kShadowMargin + toggleTop,
            toggleWidth, toggleHeight);
    }

    qreal y = kShadowMargin + kTopSectionHeight;
    separatorYs_.append(y);
    y += 1;

    struct Def {
        QString icon, text;
        bool hasSubmenu, sepAfter;
    };

    QVector<Def> defs = {
        {QStringLiteral("\U0001F399"), QStringLiteral("\u8bed\u97f3\u8f93\u5165"),   false, false},
        {QStringLiteral("\u7b26"),     QStringLiteral("\u7b26\u53f7\u5927\u5168"),   false, false},
        {QStringLiteral("\u2328"),     QStringLiteral("\u8f6f\u952e\u76d8"),         false, false},
        {QStringLiteral("\U0001F60A"), QStringLiteral("\u56fe\u7247\u8868\u60c5"),   false, false},
        {QStringLiteral("\u229E"),     QStringLiteral("\u66f4\u591a\u8f93\u5165"),   true,  true},
        {QStringLiteral("\U0001F3A8"), QStringLiteral("\u76ae\u80a4\u5546\u57ce"),   false, true},
        {QStringLiteral("Ai"),         QStringLiteral("AI\u5de5\u5177"),            true,  true},
        {QStringLiteral("\u2295"),     QStringLiteral("\u5b9a\u5236\u72b6\u6001\u680f"), false, false},
        {QStringLiteral("\u2753"),     QStringLiteral("\u5e2e\u52a9"),               true,  false},
        {QStringLiteral("\u2699"),     QStringLiteral("\u5168\u5c40\u8bbe\u7f6e"),   false, false},
    };

    for (const auto &d : defs) {
        MenuItem item;
        item.icon = d.icon;
        item.text = d.text;
        item.hasSubmenu = d.hasSubmenu;
        item.rect = QRectF(kShadowMargin, y, kMenuWidth, kItemHeight);
        items_.append(item);
        y += kItemHeight;

        if (d.sepAfter) {
            separatorYs_.append(y);
            y += 1;
        }
    }

    contentHeight_ = static_cast<int>(y - kShadowMargin + 8);
    setFixedSize(kMenuWidth + 2 * kShadowMargin,
                 contentHeight_ + 2 * kShadowMargin);
}

void StatusBarMenu::popup(const QPoint &pos) {
    QScreen *screen = QApplication::screenAt(pos);
    if (!screen) screen = QApplication::primaryScreen();

    QRect sr = screen->availableGeometry();
    int totalW = width();
    int totalH = height();
    int x = pos.x() - kShadowMargin;
    int y = pos.y() - kShadowMargin;

    if (x + totalW > sr.right() + 1)
        x = sr.right() + 1 - totalW;
    if (y + totalH > sr.bottom() + 1)
        y = pos.y() - contentHeight_ - kShadowMargin;
    if (x < sr.left()) x = sr.left();
    if (y < sr.top()) y = sr.top();

    move(x, y);
    show();
    raise();
    activateWindow();
}

int StatusBarMenu::toggleHitTest(const QPoint &pos) const {
    for (int i = 0; i < toggles_.size(); ++i) {
        if (toggles_[i].rect.contains(pos)) return i;
    }
    return -1;
}

int StatusBarMenu::itemHitTest(const QPoint &pos) const {
    for (int i = 0; i < items_.size(); ++i) {
        if (items_[i].rect.contains(pos)) return i;
    }
    return -1;
}

void StatusBarMenu::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF contentRect(kShadowMargin, kShadowMargin, kMenuWidth, contentHeight_);

    // --- Shadow layers ---
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

    // --- White background ---
    QPainterPath bgPath;
    bgPath.addRoundedRect(contentRect, kRadius, kRadius);
    p.fillPath(bgPath, QColor(255, 255, 255, 252));
    p.setPen(QPen(QColor(225, 225, 225), 0.5));
    p.drawPath(bgPath);

    // --- Clip to content ---
    p.setClipPath(bgPath);

    // --- Toggle buttons ---
    QFont charFont;
    charFont.setPointSize(20);
    charFont.setWeight(QFont::Normal);

    QFont subtitleFont;
    subtitleFont.setPointSize(8);

    for (int i = 0; i < toggles_.size(); ++i) {
        const auto &t = toggles_[i];

        if (hoveredToggle_ == i) {
            QPainterPath hp;
            hp.addRoundedRect(t.rect.adjusted(6, 3, -6, -3), 8, 8);
            p.fillPath(hp, QColor(0, 0, 0, 15));
        }

        QRectF charRect(t.rect.left(), t.rect.top(),
                        t.rect.width(), t.rect.height() * 0.62);
        p.setFont(charFont);
        p.setPen(QColor(55, 55, 55));
        p.drawText(charRect, Qt::AlignCenter, t.character);

        QRectF subRect(t.rect.left(), t.rect.top() + t.rect.height() * 0.58,
                       t.rect.width(), t.rect.height() * 0.38);
        p.setFont(subtitleFont);
        p.setPen(QColor(160, 160, 160));
        p.drawText(subRect, Qt::AlignHCenter | Qt::AlignTop, t.subtitle);
    }

    // --- Separators ---
    for (qreal sy : separatorYs_) {
        p.setPen(QPen(QColor(235, 235, 235), 1));
        p.drawLine(QPointF(kShadowMargin + kHPadding, sy + 0.5),
                   QPointF(kShadowMargin + kMenuWidth - kHPadding, sy + 0.5));
    }

    // --- Menu items ---
    QFont iconFont;
    iconFont.setPointSize(15);

    QFont textFont;
    textFont.setPointSize(12);
    textFont.setWeight(QFont::Normal);

    for (int i = 0; i < items_.size(); ++i) {
        const auto &item = items_[i];
        QRectF r = item.rect;

        if (hoveredItem_ == i) {
            QPainterPath hp;
            hp.addRoundedRect(r.adjusted(5, 2, -5, -2), 6, 6);
            p.fillPath(hp, QColor(0, 0, 0, 10));
        }

        QFont drawIconFont = iconFont;
        QColor iconColor(80, 80, 80);

        if (item.icon == QStringLiteral("Ai")) {
            drawIconFont.setPointSize(13);
            drawIconFont.setWeight(QFont::Bold);
            iconColor = QColor(40, 130, 220);
        } else if (item.icon == QStringLiteral("\u7b26")) {
            drawIconFont.setPointSize(16);
            drawIconFont.setWeight(QFont::Bold);
        }

        p.setFont(drawIconFont);
        p.setPen(iconColor);
        QRectF iconRect(r.left() + kHPadding, r.top(), 32, r.height());
        p.drawText(iconRect, Qt::AlignCenter, item.icon);

        p.setFont(textFont);
        p.setPen(QColor(50, 50, 50));
        QRectF textRect(r.left() + kHPadding + 38, r.top(),
                        r.width() - kHPadding * 2 - 56, r.height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, item.text);

        if (item.hasSubmenu) {
            QFont arrowFont;
            arrowFont.setPointSize(13);
            p.setFont(arrowFont);
            p.setPen(QColor(185, 185, 185));
            QRectF arrowRect(r.right() - kHPadding - 16, r.top(), 16, r.height());
            p.drawText(arrowRect, Qt::AlignCenter, QStringLiteral("\u203A"));
        }
    }
}

void StatusBarMenu::mouseMoveEvent(QMouseEvent *event) {
    int nt = toggleHitTest(event->pos());
    int ni = itemHitTest(event->pos());

    if (nt != hoveredToggle_ || ni != hoveredItem_) {
        hoveredToggle_ = nt;
        hoveredItem_ = ni;

        if (nt >= 0 || ni >= 0)
            setCursor(Qt::PointingHandCursor);
        else
            unsetCursor();

        update();
    }
    QWidget::mouseMoveEvent(event);
}

void StatusBarMenu::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    int ti = toggleHitTest(event->pos());
    if (ti >= 0) {
        switch (ti) {
        case 0: emit simplifiedTraditionalToggled(); break;
        case 1: emit halfFullWidthToggled(); break;
        case 2: emit chineseEnglishToggled(); break;
        case 3: emit hideStatusBarClicked(); break;
        }
        close();
        return;
    }

    int mi = itemHitTest(event->pos());
    if (mi >= 0) {
        switch (mi) {
        case 0: emit voiceInputClicked(); break;
        case 1: emit symbolsClicked(); break;
        case 2: emit softKeyboardClicked(); break;
        case 3: emit emojiClicked(); break;
        case 4: emit moreInputClicked(); break;
        case 5: emit skinStoreClicked(); break;
        case 6: emit aiToolsClicked(); break;
        case 7: emit customizeStatusBarClicked(); break;
        case 8: emit helpClicked(); break;
        case 9: emit globalSettingsClicked(); break;
        }
        close();
        return;
    }

    close();
}

void StatusBarMenu::leaveEvent(QEvent *) {
    hoveredToggle_ = -1;
    hoveredItem_ = -1;
    unsetCursor();
    update();
}
