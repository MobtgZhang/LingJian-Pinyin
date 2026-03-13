#include "about_dialog.h"
#include "theme_manager.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>

static constexpr int kDialogWidth = 320;
static constexpr int kDialogHeight = 200;
static constexpr int kRadius = 12;
static constexpr int kShadowMargin = 8;
static constexpr int kPadding = 24;

AboutDialog::AboutDialog(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setFixedSize(kDialogWidth + 2 * kShadowMargin, kDialogHeight + 2 * kShadowMargin);
}

void AboutDialog::popup(const QPoint &pos) {
    QScreen *screen = QApplication::screenAt(pos);
    if (!screen) screen = QApplication::primaryScreen();
    QRect sr = screen->availableGeometry();
    int x = pos.x() - width() / 2;
    int y = pos.y() - height() / 2;
    x = qBound(sr.left(), x, sr.right() - width());
    y = qBound(sr.top(), y, sr.bottom() - height());
    move(x, y);
    show();
    raise();
    activateWindow();
}

void AboutDialog::showCentered(QWidget *parent) {
    Q_UNUSED(parent);
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect sr = screen->availableGeometry();
        move(sr.center().x() - width() / 2, sr.center().y() - height() / 2);
    }
    show();
    raise();
    activateWindow();
}

QRectF AboutDialog::closeButtonRect() const {
    return QRectF(kShadowMargin + kDialogWidth - 36, kShadowMargin + 8, 28, 28);
}

void AboutDialog::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF contentRect(kShadowMargin, kShadowMargin, kDialogWidth, kDialogHeight);

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
    const auto &skin = ThemeManager::instance().skin();
    QPainterPath bgPath;
    bgPath.addRoundedRect(contentRect, kRadius, kRadius);
    p.fillPath(bgPath, skin.cvBackground);
    p.setPen(QPen(skin.cvBorder, 1));
    p.drawPath(bgPath);

    p.setClipPath(bgPath);

    // 关闭按钮
    QRectF closeRect = closeButtonRect();
    if (closeHovered_) {
        QPainterPath closePath;
        closePath.addRoundedRect(closeRect, 6, 6);
        p.fillPath(closePath, QColor(0, 0, 0, 25));
    }
    QFont closeFont;
    closeFont.setPointSize(16);
    closeFont.setWeight(QFont::Normal);
    p.setFont(closeFont);
    p.setPen(QColor(120, 120, 120));
    p.drawText(closeRect, Qt::AlignCenter, QStringLiteral("\u00D7"));

    // 内容区
    QRectF logoRect(kShadowMargin + kPadding, kShadowMargin + 40, 48, 48);
    QPainterPath logoPath;
    logoPath.addRoundedRect(logoRect, 10, 10);
    p.fillPath(logoPath, QColor(skin.sbLogo.red(), skin.sbLogo.green(),
                                skin.sbLogo.blue(), 30));
    p.setPen(skin.sbLogo);
    QFont logoFont;
    logoFont.setPointSize(28);
    logoFont.setWeight(QFont::Bold);
    p.setFont(logoFont);
    p.drawText(logoRect, Qt::AlignCenter, QStringLiteral("\u7075"));

    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setWeight(QFont::Bold);
    p.setFont(titleFont);
    p.setPen(skin.cvText);
    p.drawText(QRectF(kShadowMargin + kPadding + 56, kShadowMargin + 42,
                      kDialogWidth - kPadding * 2 - 56, 30),
               Qt::AlignVCenter | Qt::AlignLeft,
               QStringLiteral("\u7075\u952e\u62fc\u97f3"));

    QFont subFont;
    subFont.setPointSize(11);
    p.setFont(subFont);
    p.setPen(QColor(140, 140, 140));
    p.drawText(QRectF(kShadowMargin + kPadding + 56, kShadowMargin + 68,
                      kDialogWidth - kPadding * 2 - 56, 20),
               Qt::AlignVCenter | Qt::AlignLeft,
               QStringLiteral("LingJian Pinyin"));

    QFont descFont;
    descFont.setPointSize(11);
    p.setFont(descFont);
    p.setPen(QColor(100, 100, 100));
    p.drawText(QRectF(kShadowMargin + kPadding, kShadowMargin + 100,
                      kDialogWidth - kPadding * 2, 50),
               Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
               QStringLiteral("\u8f7b\u91cf\u7ea7\u62fc\u97f3\u8f93\u5165\u6cd5\u5e94\u7528\uff0c\u652f\u6301\u6574\u53e5\u8f93\u5165\u3002"));

    QFont verFont;
    verFont.setPointSize(10);
    p.setFont(verFont);
    p.setPen(QColor(160, 160, 160));
    p.drawText(QRectF(kShadowMargin + kPadding, kShadowMargin + kDialogHeight - 32,
                      kDialogWidth - kPadding * 2, 20),
               Qt::AlignVCenter | Qt::AlignLeft,
               QStringLiteral("\u7248\u672c 0.1.0"));
}

void AboutDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (closeButtonRect().contains(event->pos())) {
            close();
            return;
        }
        // 点击内容区外部关闭
        QRectF contentRect(kShadowMargin, kShadowMargin, kDialogWidth, kDialogHeight);
        if (!contentRect.contains(event->pos())) {
            close();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AboutDialog::mouseMoveEvent(QMouseEvent *event) {
    bool hovered = closeButtonRect().contains(event->pos());
    if (hovered != closeHovered_) {
        closeHovered_ = hovered;
        update();
    }
    QWidget::mouseMoveEvent(event);
}
