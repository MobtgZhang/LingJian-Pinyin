#include "status_bar.h"
#include "status_bar_menu.h"
#include "about_dialog.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QToolTip>
#include <QtMath>

StatusBar::StatusBar(QWidget *parent)
    : QFrame(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    rebuildButtons();
}

void StatusBar::setInputMode(InputMode mode) {
    if (inputMode_ == mode) return;
    inputMode_ = mode;
    rebuildButtons();
    update();
}

void StatusBar::setPunctuationMode(PunctuationMode mode) {
    if (punctuationMode_ == mode) return;
    punctuationMode_ = mode;
    rebuildButtons();
    update();
}

void StatusBar::setSimplifiedTraditional(SimplifiedTraditional mode) {
    if (simplifiedTraditional_ == mode) return;
    simplifiedTraditional_ = mode;
    rebuildButtons();
    update();
}

void StatusBar::setHalfFullWidth(HalfFullWidth mode) {
    if (halfFullWidth_ == mode) return;
    halfFullWidth_ = mode;
    rebuildButtons();
    update();
}

void StatusBar::applySkinColors(const QColor &bg, const QColor &border,
                                 const QColor &text, const QColor &hover,
                                 const QColor &logo, const QColor &ai,
                                 int borderRadius) {
    bgColor_ = bg;
    borderColor_ = border;
    textColor_ = text;
    hoverColor_ = hover;
    logoColor_ = logo;
    aiColor_ = ai;
    borderRadius_ = borderRadius;
    update();
}

void StatusBar::rebuildButtons() {
    buttons_.clear();

    QString modeLabel = (inputMode_ == InputMode::Chinese)
        ? QStringLiteral("\u4e2d") : QStringLiteral("\u82f1");
    // 全角=○(U+25CB) 半角=◐(U+25D0)
    QString hfLabel = (halfFullWidth_ == HalfFullWidth::Full)
        ? QStringLiteral("\u25CB") : QStringLiteral("\u25D0");

    buttons_.append({QStringLiteral("\u7075"), {}, false});
    buttons_.append({modeLabel, {}, false});
    buttons_.append({hfLabel, {}, false});
    buttons_.append({QStringLiteral("\U0001F3A4"), {}, false});
    buttons_.append({QStringLiteral("\u2328"), {}, false});
    buttons_.append({QStringLiteral("\U0001F3A8"), {}, false});
    buttons_.append({QStringLiteral("Ai"), {}, false});

    int x = kPadding;
    for (int i = 0; i < buttons_.size(); ++i) {
        if (i == 1) x += kSeparatorWidth + kPadding;
        buttons_[i].rect = QRectF(x, (kBarHeight - kButtonSize) / 2.0,
                                   kButtonSize, kButtonSize);
        x += kButtonSize + kPadding;
    }

    updateGeometry();
}

QSize StatusBar::sizeHint() const {
    if (buttons_.isEmpty()) return {200, kBarHeight};
    double right = buttons_.last().rect.right() + kPadding;
    return {static_cast<int>(qCeil(right)), kBarHeight};
}

int StatusBar::hitTest(const QPoint &pos) const {
    for (int i = 0; i < buttons_.size(); ++i) {
        if (buttons_[i].rect.contains(pos)) return i;
    }
    return -1;
}

void StatusBar::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF bgRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    QPainterPath bgPath;
    bgPath.addRoundedRect(bgRect, borderRadius_, borderRadius_);
    painter.fillPath(bgPath, bgColor_);
    painter.setPen(QPen(borderColor_, 1));
    painter.drawPath(bgPath);

    QFont baseFont;
    baseFont.setPointSize(13);
    baseFont.setWeight(QFont::Medium);

    for (int i = 0; i < buttons_.size(); ++i) {
        const auto &btn = buttons_[i];

        if (btn.hovered && i > 0) {
            QPainterPath hoverPath;
            hoverPath.addRoundedRect(btn.rect.adjusted(1, 1, -1, -1), 4, 4);
            painter.fillPath(hoverPath, hoverColor_);
        }

        if (i == 1) {
            double sepX = btn.rect.left() - kPadding / 2.0 - kSeparatorWidth / 2.0;
            painter.setPen(QPen(borderColor_, kSeparatorWidth));
            painter.drawLine(QPointF(sepX, btn.rect.top() + 4),
                             QPointF(sepX, btn.rect.bottom() - 4));
        }

        QFont drawFont = baseFont;
        QColor drawColor = textColor_;

        switch (i) {
        case 0:
            drawFont.setPointSize(14);
            drawFont.setWeight(QFont::Bold);
            drawColor = logoColor_;
            break;
        case 1:
            drawFont.setPointSize(14);
            drawFont.setWeight(QFont::Bold);
            break;
        case 2:
            drawFont.setPointSize(16);
            break;
        case 3:
            drawFont.setPointSize(13);
            break;
        case 4:
            drawFont.setPointSize(14);
            break;
        case 5:
            drawFont.setPointSize(13);
            break;
        case 6:
            drawFont.setPointSize(12);
            drawFont.setWeight(QFont::Bold);
            drawColor = aiColor_;
            break;
        default:
            break;
        }

        painter.setFont(drawFont);
        painter.setPen(drawColor);
        painter.drawText(btn.rect, Qt::AlignCenter, btn.label);
    }
}

void StatusBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        mousePressed_ = true;
        dragging_ = false;
        pressGlobalPos_ = event->globalPosition().toPoint();
        dragPosition_ = pressGlobalPos_ - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QFrame::mousePressEvent(event);
}

void StatusBar::mouseMoveEvent(QMouseEvent *event) {
    if (mousePressed_ && (event->buttons() & Qt::LeftButton)) {
        const int dragThreshold = 4;
        QPoint delta = event->globalPosition().toPoint() - pressGlobalPos_;
        if (!dragging_ && delta.manhattanLength() >= dragThreshold) {
            dragging_ = true;
            setCursor(Qt::SizeAllCursor);
        }
        if (dragging_) {
            move(event->globalPosition().toPoint() - dragPosition_);
            event->accept();
            return;
        }
    }

    int idx = hitTest(event->pos());
    bool needUpdate = false;
    for (int i = 0; i < buttons_.size(); ++i) {
        bool shouldHover = (i == idx);
        if (buttons_[i].hovered != shouldHover) {
            buttons_[i].hovered = shouldHover;
            needUpdate = true;
        }
    }
    if (needUpdate) {
        if (idx >= 0) {
            static const QStringList tooltips = {
                QStringLiteral("\u7075\u952e\u62fc\u97f3"),
                QStringLiteral("\u4e2d/\u82f1\u6587\u5207\u6362"),
                QStringLiteral("\u5168\u534a\u89d2\u5207\u6362"),
                QStringLiteral("\u8bed\u97f3\u8f93\u5165"),
                QStringLiteral("\u952e\u76d8\u5e03\u5c40"),
                QStringLiteral("\u76ae\u80a4\u8bbe\u7f6e"),
                QStringLiteral("AI \u529f\u80fd"),
            };
            if (idx < tooltips.size()) {
                QToolTip::showText(event->globalPosition().toPoint(),
                                   tooltips[idx], this);
            }
        } else {
            QToolTip::hideText();
        }
        update();
    }
    QFrame::mouseMoveEvent(event);
}

void StatusBar::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && mousePressed_) {
        mousePressed_ = false;

        if (dragging_) {
            dragging_ = false;
            unsetCursor();
            event->accept();
            return;
        }

        int idx = hitTest(event->pos());
        switch (idx) {
        case 1: {
            InputMode newMode = (inputMode_ == InputMode::Chinese)
                ? InputMode::English : InputMode::Chinese;
            setInputMode(newMode);
            emit inputModeToggled(newMode);
            break;
        }
        case 2: {
            HalfFullWidth newMode = (halfFullWidth_ == HalfFullWidth::Full)
                ? HalfFullWidth::Half : HalfFullWidth::Full;
            setHalfFullWidth(newMode);
            emit halfFullWidthToggled(newMode);
            break;
        }
        case 3:
            emit voiceInputClicked();
            break;
        case 4:
            emit keyboardClicked();
            break;
        case 5:
            emit skinClicked();
            break;
        case 6:
            emit aiClicked();
            break;
        default:
            break;
        }
        event->accept();
        return;
    }
    QFrame::mouseReleaseEvent(event);
}

void StatusBar::ensureContextMenu() {
    if (contextMenu_) return;

    contextMenu_ = new StatusBarMenu(this);

    connect(contextMenu_, &StatusBarMenu::simplifiedTraditionalToggled, this, [this]() {
        SimplifiedTraditional newMode = (simplifiedTraditional_ == SimplifiedTraditional::Simplified)
            ? SimplifiedTraditional::Traditional : SimplifiedTraditional::Simplified;
        setSimplifiedTraditional(newMode);
        emit simplifiedTraditionalToggled(newMode);
    });

    connect(contextMenu_, &StatusBarMenu::halfFullWidthToggled, this, [this]() {
        HalfFullWidth newMode = (halfFullWidth_ == HalfFullWidth::Full)
            ? HalfFullWidth::Half : HalfFullWidth::Full;
        setHalfFullWidth(newMode);
        emit halfFullWidthToggled(newMode);
    });

    connect(contextMenu_, &StatusBarMenu::chineseEnglishToggled, this, [this]() {
        InputMode newMode = (inputMode_ == InputMode::Chinese)
            ? InputMode::English : InputMode::Chinese;
        setInputMode(newMode);
        emit inputModeToggled(newMode);
    });

    connect(contextMenu_, &StatusBarMenu::hideStatusBarClicked,
            this, [this]() { emit hideRequested(); });
    connect(contextMenu_, &StatusBarMenu::voiceInputClicked,
            this, &StatusBar::voiceInputClicked);
    connect(contextMenu_, &StatusBarMenu::softKeyboardClicked,
            this, &StatusBar::keyboardClicked);
    connect(contextMenu_, &StatusBarMenu::skinStoreClicked,
            this, &StatusBar::skinClicked);
    connect(contextMenu_, &StatusBarMenu::aiToolsClicked,
            this, &StatusBar::aiClicked);
    connect(contextMenu_, &StatusBarMenu::aboutClicked, this, [this]() {
        AboutDialog *about = new AboutDialog(this);
        about->setAttribute(Qt::WA_DeleteOnClose);
        about->showCentered(window());
    });
}

void StatusBar::contextMenuEvent(QContextMenuEvent *event) {
    showContextMenuAt(event->globalPos());
}

void StatusBar::showContextMenuAt(const QPoint &globalPos) {
    ensureContextMenu();
    contextMenu_->popup(globalPos);
}

void StatusBar::requestHide() {
    emit hideRequested();
}
