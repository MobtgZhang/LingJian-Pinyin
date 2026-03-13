#include "skin_selector.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QApplication>
#include <QScreen>
#include <QtMath>

SkinSelector::SkinSelector(QWidget *parent)
    : QWidget(parent) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    scrollAnim_ = new QPropertyAnimation(this, "scrollOffset", this);
    scrollAnim_->setDuration(250);
    scrollAnim_->setEasingCurve(QEasingCurve::OutCubic);

    addBuiltinSkins();
}

void SkinSelector::addBuiltinSkins() {
    skins_.clear();

    skins_.append({
        "light", "亮色",
        QColor(255, 255, 255), QColor(210, 210, 210),
        QColor(51, 51, 51), QColor(74, 144, 217),
        QColor(255, 120, 0), false
    });

    skins_.append({
        "dark", "暗色",
        QColor(30, 30, 30), QColor(80, 80, 80),
        QColor(240, 240, 240), QColor(0, 180, 255),
        QColor(255, 140, 26), false
    });

    skins_.append({
        "blue", "海蓝",
        QColor(25, 55, 110), QColor(50, 100, 200),
        QColor(220, 235, 255), QColor(100, 220, 255),
        QColor(255, 180, 50), false
    });

    skins_.append({
        "green", "草绿",
        QColor(34, 85, 51), QColor(46, 125, 70),
        QColor(230, 255, 235), QColor(120, 230, 150),
        QColor(255, 220, 80), false
    });

    skins_.append({
        "purple", "星紫",
        QColor(48, 30, 80), QColor(90, 60, 140),
        QColor(235, 220, 255), QColor(180, 130, 255),
        QColor(255, 170, 100), false
    });

    skins_.append({
        "rose", "玫瑰",
        QColor(100, 20, 50), QColor(160, 40, 80),
        QColor(255, 225, 235), QColor(255, 120, 170),
        QColor(255, 200, 50), false
    });

    skins_.append({
        "sunset", "落霞",
        QColor(80, 35, 10), QColor(160, 70, 20),
        QColor(255, 240, 220), QColor(255, 160, 50),
        QColor(255, 100, 60), false
    });

    skins_.append({
        "ice", "冰川",
        QColor(220, 240, 250), QColor(180, 210, 230),
        QColor(30, 60, 80), QColor(40, 140, 200),
        QColor(0, 180, 220), false
    });

    currentSkinId_ = "light";
    rebuildLayout();
}

void SkinSelector::addSkin(const SkinPreview &skin) {
    skins_.append(skin);
    rebuildLayout();
}

void SkinSelector::setCurrentSkin(const QString &id) {
    currentSkinId_ = id;
    update();
}

void SkinSelector::setScrollOffset(qreal offset) {
    scrollOffset_ = qBound(0.0, offset, maxScrollOffset());
    update();
}

qreal SkinSelector::maxScrollOffset() const {
    int totalWidth = kPanelPadding +
        skins_.size() * (kCardWidth + kCardSpacing) +
        kCardWidth + kCardSpacing + kPanelPadding;
    int viewWidth = width() - 2 * kShadowMargin;
    return qMax(0.0, static_cast<qreal>(totalWidth - viewWidth));
}

void SkinSelector::rebuildLayout() {
    int viewWidth = kPanelPadding +
        (skins_.size() + 1) * (kCardWidth + kCardSpacing) +
        kPanelPadding;
    int displayWidth = qMin(viewWidth + 2 * kShadowMargin,
                            600 + 2 * kShadowMargin);
    int displayHeight = kTitleHeight + kPanelHeight + 2 * kShadowMargin;
    setFixedSize(displayWidth, displayHeight);
}

void SkinSelector::popup(const QPoint &pos) {
    QScreen *screen = QApplication::screenAt(pos);
    if (!screen) screen = QApplication::primaryScreen();

    QRect sr = screen->availableGeometry();
    int w = width();
    int h = height();
    int x = pos.x() - w / 2;
    int y = pos.y() - h - 8;

    if (x + w > sr.right() + 1) x = sr.right() + 1 - w;
    if (x < sr.left()) x = sr.left();
    if (y < sr.top()) y = pos.y() + 8;

    move(x, y);
    scrollOffset_ = 0;
    show();
    raise();
    activateWindow();
}

void SkinSelector::animateScroll(qreal target) {
    target = qBound(0.0, target, maxScrollOffset());
    scrollAnim_->stop();
    scrollAnim_->setStartValue(scrollOffset_);
    scrollAnim_->setEndValue(target);
    scrollAnim_->start();
}

int SkinSelector::hitTestCard(const QPoint &pos) const {
    qreal contentLeft = kShadowMargin + kPanelPadding - scrollOffset_;
    qreal cardY = kShadowMargin + kTitleHeight +
                  (kPanelHeight - kCardHeight) / 2.0;

    for (int i = 0; i < skins_.size(); ++i) {
        qreal cardX = contentLeft + i * (kCardWidth + kCardSpacing);
        QRectF cardRect(cardX, cardY, kCardWidth, kCardHeight);
        if (cardRect.contains(pos)) return i;
    }
    return -1;
}

void SkinSelector::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF contentRect(kShadowMargin, kShadowMargin,
                       width() - 2 * kShadowMargin,
                       height() - 2 * kShadowMargin);

    // 阴影
    struct ShadowLayer { qreal spread; int alpha; };
    ShadowLayer layers[] = {
        {7.0, 3}, {5.0, 6}, {3.5, 10}, {2.0, 16}, {0.5, 22},
    };
    for (const auto &l : layers) {
        QPainterPath sp;
        sp.addRoundedRect(contentRect.adjusted(-l.spread, -l.spread,
                                                l.spread, l.spread),
                          kRadius + l.spread, kRadius + l.spread);
        p.fillPath(sp, QColor(0, 0, 0, l.alpha));
    }

    // 背景
    QPainterPath bgPath;
    bgPath.addRoundedRect(contentRect, kRadius, kRadius);
    p.fillPath(bgPath, QColor(255, 255, 255, 252));
    p.setPen(QPen(QColor(225, 225, 225), 0.5));
    p.drawPath(bgPath);

    p.setClipPath(bgPath);

    // 标题栏
    QFont titleFont;
    titleFont.setPointSize(13);
    titleFont.setWeight(QFont::Bold);
    p.setFont(titleFont);
    p.setPen(QColor(50, 50, 50));
    QRectF titleRect(kShadowMargin + kPanelPadding, kShadowMargin,
                     200, kTitleHeight);
    p.drawText(titleRect, Qt::AlignVCenter | Qt::AlignLeft,
               QStringLiteral("选择皮肤"));

    // 分隔线
    qreal sepY = kShadowMargin + kTitleHeight;
    p.setPen(QPen(QColor(235, 235, 235), 1));
    p.drawLine(QPointF(kShadowMargin + kPanelPadding, sepY),
               QPointF(width() - kShadowMargin - kPanelPadding, sepY));

    // 滑动区域裁剪
    QRectF slideArea(kShadowMargin, sepY + 1,
                     width() - 2 * kShadowMargin,
                     kPanelHeight - 1);
    p.setClipRect(slideArea);

    qreal contentLeft = kShadowMargin + kPanelPadding - scrollOffset_;
    qreal cardY = kShadowMargin + kTitleHeight +
                  (kPanelHeight - kCardHeight) / 2.0;

    for (int i = 0; i < skins_.size(); ++i) {
        const auto &skin = skins_[i];
        qreal cardX = contentLeft + i * (kCardWidth + kCardSpacing);
        QRectF cardRect(cardX, cardY, kCardWidth, kCardHeight);

        if (cardRect.right() < kShadowMargin ||
            cardRect.left() > width() - kShadowMargin) {
            continue;
        }

        bool isCurrent = (skin.id == currentSkinId_);
        bool isHovered = (hoveredCard_ == i);

        // 卡片背景
        QPainterPath cardPath;
        cardPath.addRoundedRect(cardRect, 8, 8);
        p.fillPath(cardPath, skin.background);

        // 卡片内的模拟效果
        QRectF previewBar(cardRect.left() + 8, cardRect.top() + 8,
                          cardRect.width() - 16, 24);
        QPainterPath barPath;
        barPath.addRoundedRect(previewBar, 4, 4);
        p.fillPath(barPath, QColor(skin.text.red(), skin.text.green(),
                                    skin.text.blue(), 20));

        QFont previewFont;
        previewFont.setPointSize(9);
        p.setFont(previewFont);
        p.setPen(skin.text);
        p.drawText(previewBar.adjusted(6, 0, 0, 0),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QStringLiteral("拼音输入"));

        // 模拟候选词
        QRectF candBar(cardRect.left() + 8, cardRect.top() + 38,
                       cardRect.width() - 16, 20);
        QPainterPath candPath;
        candPath.addRoundedRect(candBar, 3, 3);
        p.fillPath(candPath, QColor(skin.highlight.red(), skin.highlight.green(),
                                     skin.highlight.blue(), 40));

        previewFont.setPointSize(8);
        p.setFont(previewFont);
        p.setPen(skin.highlight);
        p.drawText(candBar.adjusted(4, 0, 0, 0),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QStringLiteral("1.灵  2.键  3.好"));

        // 模拟高亮圆点（accent色）
        qreal dotX = cardRect.left() + cardRect.width() - 18;
        qreal dotY = cardRect.top() + 12;
        p.setBrush(skin.accent);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(dotX, dotY), 5, 5);

        // 卡片名称
        QFont nameFont;
        nameFont.setPointSize(10);
        nameFont.setWeight(QFont::Medium);
        p.setFont(nameFont);
        p.setPen(skin.text);
        QRectF nameRect(cardRect.left(), cardRect.bottom() - 26,
                        cardRect.width(), 22);
        p.drawText(nameRect, Qt::AlignCenter, skin.name);

        // 选中高亮边框
        if (isCurrent) {
            p.setPen(QPen(skin.highlight, 2.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(cardRect.adjusted(1, 1, -1, -1), 8, 8);

            // 选中标记 ✓
            QFont checkFont;
            checkFont.setPointSize(12);
            checkFont.setWeight(QFont::Bold);
            p.setFont(checkFont);
            p.setPen(skin.highlight);
            p.drawText(QRectF(cardRect.right() - 24, cardRect.top() + 2, 20, 20),
                       Qt::AlignCenter, QStringLiteral("✓"));
        }

        // 悬停边框
        if (isHovered && !isCurrent) {
            p.setPen(QPen(QColor(skin.highlight.red(), skin.highlight.green(),
                                  skin.highlight.blue(), 120), 1.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(cardRect.adjusted(1, 1, -1, -1), 8, 8);
        }

        // 卡片外边框
        p.setPen(QPen(skin.border, 0.5));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(cardRect, 8, 8);
    }

    // 「加载自定义」按钮卡片
    qreal loadX = contentLeft + skins_.size() * (kCardWidth + kCardSpacing);
    loadButtonRect_ = QRectF(loadX, cardY, kCardWidth, kCardHeight);

    if (loadButtonRect_.right() >= kShadowMargin &&
        loadButtonRect_.left() <= width() - kShadowMargin) {
        QPainterPath loadPath;
        loadPath.addRoundedRect(loadButtonRect_, 8, 8);

        QColor loadBg = loadButtonHovered_ ?
            QColor(245, 245, 250) : QColor(250, 250, 252);
        p.fillPath(loadPath, loadBg);

        p.setPen(QPen(QColor(200, 200, 210), 1, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(loadButtonRect_.adjusted(2, 2, -2, -2), 7, 7);

        QFont plusFont;
        plusFont.setPointSize(28);
        plusFont.setWeight(QFont::Light);
        p.setFont(plusFont);
        p.setPen(QColor(160, 160, 180));
        p.drawText(QRectF(loadButtonRect_.left(), loadButtonRect_.top(),
                          loadButtonRect_.width(), loadButtonRect_.height() - 20),
                   Qt::AlignCenter, QStringLiteral("+"));

        QFont loadFont;
        loadFont.setPointSize(9);
        p.setFont(loadFont);
        p.setPen(QColor(120, 120, 140));
        p.drawText(QRectF(loadButtonRect_.left(), loadButtonRect_.bottom() - 28,
                          loadButtonRect_.width(), 22),
                   Qt::AlignCenter, QStringLiteral("加载皮肤"));
    }

    // 滚动指示器
    p.setClipPath(bgPath);
    qreal maxOff = maxScrollOffset();
    if (maxOff > 0) {
        qreal viewW = width() - 2 * kShadowMargin - 2 * kPanelPadding;
        qreal indicatorW = qMax(30.0, viewW * viewW /
            (viewW + maxOff));
        qreal indicatorX = kShadowMargin + kPanelPadding +
            (viewW - indicatorW) * (scrollOffset_ / maxOff);
        qreal indicatorY = contentRect.bottom() - 6;

        QPainterPath indPath;
        indPath.addRoundedRect(QRectF(indicatorX, indicatorY, indicatorW, 3),
                               1.5, 1.5);
        p.fillPath(indPath, QColor(0, 0, 0, 40));
    }

    // 左右渐变遮罩
    if (scrollOffset_ > 0) {
        QLinearGradient leftGrad(slideArea.left(), 0,
                                  slideArea.left() + 30, 0);
        leftGrad.setColorAt(0, QColor(255, 255, 255, 200));
        leftGrad.setColorAt(1, QColor(255, 255, 255, 0));
        p.fillRect(QRectF(slideArea.left(), slideArea.top(), 30, slideArea.height()),
                   leftGrad);
    }
    if (scrollOffset_ < maxOff) {
        QLinearGradient rightGrad(slideArea.right() - 30, 0,
                                   slideArea.right(), 0);
        rightGrad.setColorAt(0, QColor(255, 255, 255, 0));
        rightGrad.setColorAt(1, QColor(255, 255, 255, 200));
        p.fillRect(QRectF(slideArea.right() - 30, slideArea.top(),
                          30, slideArea.height()),
                   rightGrad);
    }
}

void SkinSelector::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    dragging_ = false;
    dragStartPos_ = event->pos();
    dragStartOffset_ = scrollOffset_;
    event->accept();
}

void SkinSelector::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        int dx = event->pos().x() - dragStartPos_.x();
        if (qAbs(dx) > 5) {
            dragging_ = true;
            setCursor(Qt::ClosedHandCursor);
            setScrollOffset(dragStartOffset_ - dx);
        }
        event->accept();
        return;
    }

    int idx = hitTestCard(event->pos());
    bool onLoad = loadButtonRect_.contains(event->pos());

    if (idx != hoveredCard_ || onLoad != loadButtonHovered_) {
        hoveredCard_ = idx;
        loadButtonHovered_ = onLoad;
        setCursor((idx >= 0 || onLoad) ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void SkinSelector::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    if (dragging_) {
        dragging_ = false;
        unsetCursor();
        event->accept();
        return;
    }

    int idx = hitTestCard(event->pos());
    if (idx >= 0 && idx < skins_.size()) {
        currentSkinId_ = skins_[idx].id;
        emit skinSelected(skins_[idx].id);
        update();
        close();
        return;
    }

    if (loadButtonRect_.contains(event->pos())) {
        emit loadCustomSkinClicked();
        close();
        return;
    }

    event->accept();
}

void SkinSelector::wheelEvent(QWheelEvent *event) {
    qreal delta = -event->angleDelta().y() * 0.8;
    animateScroll(scrollOffset_ + delta);
    event->accept();
}

void SkinSelector::leaveEvent(QEvent *) {
    hoveredCard_ = -1;
    loadButtonHovered_ = false;
    unsetCursor();
    update();
}
