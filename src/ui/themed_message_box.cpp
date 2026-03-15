#include "themed_message_box.h"
#include "theme_manager.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>

namespace {

static constexpr int kDialogWidth = 360;
static constexpr int kDialogHeight = 160;
static constexpr int kRadius = 12;
static constexpr int kShadowMargin = 8;
static constexpr int kPadding = 24;

class ThemedMessageDialog : public QDialog {
public:
    ThemedMessageDialog(const QString& title, const QString& text, QWidget* parent = nullptr)
        : QDialog(parent) {
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(kDialogWidth + 2 * kShadowMargin, kDialogHeight + 2 * kShadowMargin);

        auto* central = new QWidget(this);
        central->setGeometry(kShadowMargin, kShadowMargin, kDialogWidth, kDialogHeight);
        central->setAttribute(Qt::WA_TranslucentBackground);

        auto* layout = new QVBoxLayout(central);
        layout->setContentsMargins(kPadding, kPadding + 28, kPadding, kPadding);
        layout->setSpacing(16);

        titleLabel_ = new QLabel(title);
        titleLabel_->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
        layout->addWidget(titleLabel_);

        textLabel_ = new QLabel(text);
        textLabel_->setWordWrap(true);
        textLabel_->setStyleSheet(QStringLiteral("font-size: 13px;"));
        layout->addWidget(textLabel_, 1);

        auto* btnLayout = new QHBoxLayout;
        btnLayout->addStretch();
        okButton_ = new QPushButton(QStringLiteral("确定"));
        okButton_->setCursor(Qt::PointingHandCursor);
        okButton_->setFixedHeight(36);
        okButton_->setMinimumWidth(100);
        connect(okButton_, &QPushButton::clicked, this, &QDialog::accept);
        btnLayout->addWidget(okButton_);
        btnLayout->addStretch();
        layout->addLayout(btnLayout);
    }

    void updateSkinStyles() {
        const auto& skin = ThemeManager::instance().skin();
        const QString textColor = skin.cvText.name();
        const QString accent = skin.sbLogo.name();
        const bool isDark = skin.cvBackground.lightness() < 128;
        QColor accentHover = QColor(accent).lighter(isDark ? 115 : 110);
        QColor accentPressed = QColor(accent).darker(isDark ? 115 : 110);
        if (titleLabel_) titleLabel_->setStyleSheet(
            QStringLiteral("font-weight: bold; font-size: 14px; color: %1;").arg(textColor));
        if (textLabel_) textLabel_->setStyleSheet(
            QStringLiteral("font-size: 13px; color: %1;").arg(textColor));
        if (okButton_) {
            okButton_->setStyleSheet(QStringLiteral(
                "QPushButton { background: %1; border: none; border-radius: 6px; color: white; }"
                "QPushButton:hover { background: %2; color: white; }"
                "QPushButton:pressed { background: %3; color: white; }"
            ).arg(accent, accentHover.name(), accentPressed.name()));
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::TextAntialiasing, true);
        QRectF contentRect(kShadowMargin, kShadowMargin, kDialogWidth, kDialogHeight);

        struct ShadowLayer { qreal spread; int alpha; };
        ShadowLayer layers[] = {{7.0, 3}, {5.0, 6}, {3.5, 10}, {2.0, 16}, {0.5, 22}};
        for (const auto& l : layers) {
            QPainterPath sp;
            sp.addRoundedRect(contentRect.adjusted(-l.spread, -l.spread, l.spread, l.spread),
                              kRadius + l.spread, kRadius + l.spread);
            p.fillPath(sp, QColor(0, 0, 0, l.alpha));
        }

        const auto& skin = ThemeManager::instance().skin();
        QPainterPath bgPath;
        bgPath.addRoundedRect(contentRect, kRadius, kRadius);
        p.fillPath(bgPath, skin.cvBackground);
        p.setPen(QPen(skin.cvBorder, 1));
        p.drawPath(bgPath);
    }

    void showEvent(QShowEvent* e) override {
        updateSkinStyles();
        QDialog::showEvent(e);
    }

private:
    QLabel* titleLabel_ = nullptr;
    QLabel* textLabel_ = nullptr;
    QPushButton* okButton_ = nullptr;
};

} // namespace

void ThemedMessageBox::warning(QWidget* parent, const QString& title, const QString& text,
                               QWidget* voiceDialog) {
    if (voiceDialog) {
        voiceDialog->setProperty("suppressCloseOnFocusLoss", true);
    }

    ThemedMessageDialog dlg(title, text, parent);
    if (parent && parent->window()) {
        QPoint c = parent->window()->geometry().center();
        QScreen* screen = QApplication::screenAt(c);
        if (!screen) screen = QApplication::primaryScreen();
        if (screen) {
            QRect sr = screen->availableGeometry();
            int x = c.x() - dlg.width() / 2;
            int y = c.y() - dlg.height() / 2;
            x = qBound(sr.left(), x, sr.right() - dlg.width());
            y = qBound(sr.top(), y, sr.bottom() - dlg.height());
            dlg.move(x, y);
        }
    }

    QObject::connect(&dlg, &QDialog::finished, &dlg, [voiceDialog]() {
        if (voiceDialog) {
            voiceDialog->setProperty("suppressCloseOnFocusLoss", false);
        }
    });

    dlg.exec();
}
