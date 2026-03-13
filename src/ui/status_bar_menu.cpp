#include "status_bar_menu.h"
#include "help_submenu.h"
#include "theme_manager.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QApplication>
#include <QScreen>
#include <QCursor>

StatusBarMenu::StatusBarMenu(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    hoverSubmenuTimer_.setSingleShot(true);
    connect(&hoverSubmenuTimer_, &QTimer::timeout, this, &StatusBarMenu::showHelpSubmenu);
    submenuCloseTimer_.setSingleShot(true);
    connect(&submenuCloseTimer_, &QTimer::timeout, this, &StatusBarMenu::closeHelpSubmenuIfNeeded);
    connect(&checkCursorTimer_, &QTimer::timeout, this, &StatusBarMenu::checkCursorOverMainMenu);
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
    const auto &skin = ThemeManager::instance().skin();
    const bool isDark = skin.cvBackground.lightness() < 128;
    const QColor hoverOverlay = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 35);
    const QColor subTextColor = isDark ? QColor(180, 180, 180) : QColor(160, 160, 160);
    const QColor arrowColor = isDark ? QColor(140, 140, 140) : QColor(185, 185, 185);

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

    // --- Background (使用当前皮肤) ---
    QPainterPath bgPath;
    bgPath.addRoundedRect(contentRect, kRadius, kRadius);
    p.fillPath(bgPath, skin.cvBackground);
    p.setPen(QPen(skin.cvBorder, 0.5));
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
            hp.addRoundedRect(t.rect.adjusted(2, 1, -2, -1), 8, 8);
            p.fillPath(hp, hoverOverlay);
        }

        QRectF charRect(t.rect.left(), t.rect.top(),
                        t.rect.width(), t.rect.height() * 0.62);
        p.setFont(charFont);
        p.setPen(skin.cvText);
        p.drawText(charRect, Qt::AlignCenter, t.character);

        QRectF subRect(t.rect.left(), t.rect.top() + t.rect.height() * 0.58,
                       t.rect.width(), t.rect.height() * 0.38);
        p.setFont(subtitleFont);
        p.setPen(subTextColor);
        p.drawText(subRect, Qt::AlignHCenter | Qt::AlignTop, t.subtitle);
    }

    // --- Separators ---
    p.setPen(QPen(skin.cvSeparator, 1));
    for (qreal sy : separatorYs_) {
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
            hp.addRoundedRect(r.adjusted(2, 1, -2, -1), 6, 6);
            p.fillPath(hp, hoverOverlay);
        }

        QFont drawIconFont = iconFont;
        QColor iconColor = skin.cvText;

        if (item.icon == QStringLiteral("Ai")) {
            drawIconFont.setPointSize(13);
            drawIconFont.setWeight(QFont::Bold);
            iconColor = skin.sbAi;
        } else if (item.icon == QStringLiteral("\u7b26")) {
            drawIconFont.setPointSize(16);
            drawIconFont.setWeight(QFont::Bold);
        }

        p.setFont(drawIconFont);
        p.setPen(iconColor);
        QRectF iconRect(r.left() + kHPadding, r.top(), 32, r.height());
        p.drawText(iconRect, Qt::AlignCenter, item.icon);

        p.setFont(textFont);
        p.setPen(skin.cvText);
        QRectF textRect(r.left() + kHPadding + 38, r.top(),
                        r.width() - kHPadding * 2 - 56, r.height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, item.text);

        if (item.hasSubmenu) {
            QFont arrowFont;
            arrowFont.setPointSize(13);
            p.setFont(arrowFont);
            p.setPen(arrowColor);
            QRectF arrowRect(r.right() - kHPadding - 16, r.top(), 16, r.height());
            p.drawText(arrowRect, Qt::AlignCenter, QStringLiteral("\u203A"));
        }
    }
}

void StatusBarMenu::enterEvent(QEnterEvent *event) {
    // 鼠标从子菜单滑回主菜单时，若停在其他选项上则关闭二级菜单
    QPoint pos = event->position().toPoint();
    int nt = toggleHitTest(pos);
    int ni = itemHitTest(pos);
    if (helpSubmenu_ && helpSubmenu_->isVisible() && (nt >= 0 || (ni >= 0 && ni != 8))) {
        helpSubmenu_->close();
        helpSubmenu_ = nullptr;
    }
    if (nt >= 0 || ni >= 0) {
        hoveredToggle_ = nt;
        hoveredItem_ = ni;
        if (ni == 8) {
            hoverSubmenuTimer_.start(100);
        } else {
            hoverSubmenuTimer_.stop();
            if (helpSubmenu_ && helpSubmenu_->isVisible()) {
                helpSubmenu_->close();
                helpSubmenu_ = nullptr;
            }
        }
        setCursor(Qt::PointingHandCursor);
    }
    update();
    QWidget::enterEvent(event);
}

void StatusBarMenu::mouseMoveEvent(QMouseEvent *event) {
    int nt = toggleHitTest(event->pos());
    int ni = itemHitTest(event->pos());

    if (nt != hoveredToggle_ || ni != hoveredItem_) {
        hoveredToggle_ = nt;
        hoveredItem_ = ni;

        // 滑到「帮助」弹出二级菜单（关于灵键），滑到其他选项关闭二级菜单
        if (ni == 8) {
            hoverSubmenuTimer_.start(100);
        } else {
            hoverSubmenuTimer_.stop();
            if (helpSubmenu_ && helpSubmenu_->isVisible()) {
                helpSubmenu_->close();
                helpSubmenu_ = nullptr;
            }
        }

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
        if (mi == 8) {
            // 帮助：二级菜单已通过悬停显示，点击时若子菜单已打开则不做处理（点击会传递到子菜单）
            // 若用户快速点击未悬停，则立即显示子菜单
            if (!helpSubmenu_ || !helpSubmenu_->isVisible()) {
                showHelpSubmenu();
            }
            return;
        }
        switch (mi) {
        case 0: emit voiceInputClicked(); break;
        case 1: emit symbolsClicked(); break;
        case 2: emit softKeyboardClicked(); break;
        case 3: emit emojiClicked(); break;
        case 4: emit moreInputClicked(); break;
        case 5: emit skinStoreClicked(); break;
        case 6: emit aiToolsClicked(); break;
        case 7: emit customizeStatusBarClicked(); break;
        case 9: emit globalSettingsClicked(); break;
        default: break;
        }
        close();
        return;
    }

    close();
}

void StatusBarMenu::showHelpSubmenu() {
    if (helpSubmenu_ && helpSubmenu_->isVisible())
        return;
    if (helpSubmenu_) {
        helpSubmenu_->close();
        helpSubmenu_ = nullptr;
    }
    helpSubmenu_ = new HelpSubmenu(this);
    connect(helpSubmenu_, &HelpSubmenu::aboutClicked, this, [this]() {
        emit aboutClicked();
        close();
    });
    connect(helpSubmenu_, &QWidget::destroyed, this, [this]() {
        helpSubmenu_ = nullptr;
        checkCursorTimer_.stop();
    });
    QRectF helpRect = items_[8].rect;
    QPoint submenuPos = mapToGlobal(QPoint(static_cast<int>(helpRect.right() - 4),
                                           static_cast<int>(helpRect.top())));
    helpSubmenu_->popup(submenuPos);
    checkCursorTimer_.start(50);
}

void StatusBarMenu::closeHelpSubmenuIfNeeded() {
    if (!helpSubmenu_ || !helpSubmenu_->isVisible())
        return;
    QPoint gp = QCursor::pos();
    QRect subGeom = helpSubmenu_->geometry();
    if (subGeom.contains(gp))
        return;
    checkCursorTimer_.stop();
    helpSubmenu_->close();
    helpSubmenu_ = nullptr;
}

void StatusBarMenu::checkCursorOverMainMenu() {
    if (!helpSubmenu_ || !helpSubmenu_->isVisible())
        return;
    QPoint gp = QCursor::pos();
    QPoint localPos = mapFromGlobal(gp);
    if (!rect().contains(localPos))
        return;
    int nt = toggleHitTest(localPos);
    int ni = itemHitTest(localPos);
    if (nt >= 0 || (ni >= 0 && ni != 8)) {
        checkCursorTimer_.stop();
        helpSubmenu_->close();
        helpSubmenu_ = nullptr;
    }
}

void StatusBarMenu::leaveEvent(QEvent *) {
    hoveredToggle_ = -1;
    hoveredItem_ = -1;
    hoverSubmenuTimer_.stop();
    if (helpSubmenu_ && helpSubmenu_->isVisible()) {
        submenuCloseTimer_.start(150);
    }
    unsetCursor();
    update();
}
