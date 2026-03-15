#include "voice_input_dialog.h"
#include "theme_manager.h"
#include "vosk_speech_engine.h"

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QDir>
#include <QListView>
#include <QShowEvent>
#include <QTimer>
#include <QFocusEvent>
#include <QtConcurrent>
#include <QFutureWatcher>

// 下拉弹出层由 Qt 内部创建且为独立窗口，样式表无法从 QComboBox 继承，
// 故用自定义 ComboBox 在 showPopup 后对弹出容器直接设置主题样式，去除黑边。
class ThemedComboBox : public QComboBox {
public:
    explicit ThemedComboBox(QWidget *parent = nullptr) : QComboBox(parent) {}

protected:
    void showPopup() override {
        QComboBox::showPopup();
        // 弹出层在 showPopup 后可能尚未完成布局，延迟一帧再应用样式
        QTimer::singleShot(0, this, [this]() {
            if (!view()) return;
            QWidget *popup = view()->parentWidget();
            if (!popup || popup == this)
                popup = view()->window();
            if (!popup || popup == this) return;
            const auto &skin = ThemeManager::instance().skin();
            const QString bg = skin.cvBackground.name();
            const QString border = skin.cvBorder.name();
            const QString text = skin.cvText.name();
            const bool isDark = skin.cvBackground.lightness() < 128;
            const QString hoverRgba = isDark
                ? QStringLiteral("rgba(255, 255, 255, 0.16)")
                : QStringLiteral("rgba(0, 0, 0, 0.14)");
            popup->setStyleSheet(QStringLiteral(
                "QWidget { background: %1; border: 1px solid %2; border-radius: 6px; }"
                "QAbstractItemView { outline: none; border: none; background: %1; color: %3; }"
                "QAbstractItemView::item { min-height: 28px; padding: 4px 12px; }"
                "QAbstractItemView::item:hover, QAbstractItemView::item:selected { background: %4; }"
                "QScrollBar:vertical { background: %1; border: none; width: 8px; border-radius: 4px; }"
                "QScrollBar::handle:vertical { background: %2; border-radius: 4px; min-height: 24px; }"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
            ).arg(bg, border, text, hoverRgba));
        });
    }
};

VoiceInputDialog::VoiceInputDialog(QWidget *parent)
    : QWidget(parent) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setFixedSize(kDialogWidth + 2 * kShadowMargin, kDialogHeight + 2 * kShadowMargin);
    setupUi();
    connect(&ThemeManager::instance(), &ThemeManager::skinChanged,
            this, &VoiceInputDialog::updateSkinStyles);
}

void VoiceInputDialog::setupUi() {
    auto *central = new QWidget(this);
    central->setGeometry(kShadowMargin, kShadowMargin, kDialogWidth, kDialogHeight);
    central->setAttribute(Qt::WA_TranslucentBackground);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(24, 56, 24, 20);
    mainLayout->setSpacing(16);

    // 提示文字
    hintLabel_ = new QLabel(QStringLiteral("点击麦克风开始说话，再次点击停止 · 快捷键 F2"));
    hintLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(hintLabel_, 0, Qt::AlignCenter);

    // 麦克风按钮容器（居中）
    auto *micContainer = new QWidget;
    auto *micLayout = new QVBoxLayout(micContainer);
    micLayout->setAlignment(Qt::AlignCenter);
    micLayout->setContentsMargins(0, 10, 0, 10);

    micButton_ = new QPushButton;
    micButton_->setFixedSize(kMicButtonSize, kMicButtonSize);
    micButton_->setCursor(Qt::PointingHandCursor);
    micButton_->setIconSize(QSize(40, 40));
    connect(micButton_, &QPushButton::clicked, this, &VoiceInputDialog::onMicButtonClicked);
    micLayout->addWidget(micButton_, 0, Qt::AlignCenter);

    mainLayout->addWidget(micContainer, 1);

    // 加载 SVG 图标（麦克风 / 停止）
    const QString micSvg = findSvgPath(QStringLiteral("voice_mic_icon.svg"));
    const QString stopSvg = findSvgPath(QStringLiteral("voice_stop_icon.svg"));
    if (!micSvg.isEmpty() && !stopSvg.isEmpty()) {
        micIcon_ = QIcon(micSvg);
        stopIcon_ = QIcon(stopSvg);
        svgIconsLoaded_ = !micIcon_.isNull() && !stopIcon_.isNull();
    }
    updateMicButtonIcon();

    // 底部：语言 + 设备
    auto *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(16);

    languageCombo_ = new ThemedComboBox(central);
    languageCombo_->setView(new QListView(languageCombo_));
    languageCombo_->addItem(QStringLiteral("普通话"), static_cast<int>(Language::Mandarin));
    languageCombo_->addItem(QStringLiteral("英语"), static_cast<int>(Language::English));
    connect(languageCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VoiceInputDialog::onLanguageChanged);
    bottomLayout->addWidget(languageCombo_);

    bottomLayout->addStretch();

    deviceCombo_ = new ThemedComboBox(central);
    deviceCombo_->setView(new QListView(deviceCombo_));
    connect(deviceCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VoiceInputDialog::onDeviceChanged);
    deviceCombo_->addItem(QStringLiteral("加载中..."), QByteArray());
    QTimer::singleShot(0, this, &VoiceInputDialog::refreshAudioDevicesAsync);
    bottomLayout->addWidget(deviceCombo_);

    mainLayout->addLayout(bottomLayout);

    f2Shortcut_ = new QShortcut(QKeySequence(Qt::Key_F2), this);
    connect(f2Shortcut_, &QShortcut::activated, this, &VoiceInputDialog::onMicButtonClicked);

    updateSkinStyles();
}

QString VoiceInputDialog::findSvgPath(const QString &fileName) const {
    const QStringList basePaths = {
        QApplication::applicationDirPath() + QStringLiteral("/../../data"),
        QApplication::applicationDirPath() + QStringLiteral("/../data"),
        QApplication::applicationDirPath() + QStringLiteral("/data"),
        QStringLiteral("data"),
        QStringLiteral("../data"),
    };
    for (const auto &base : basePaths) {
        QString p = base + QChar('/') + fileName;
        if (QFile::exists(p)) return QDir(p).absolutePath();
    }
    return {};
}

void VoiceInputDialog::updateSkinStyles() {
    const auto &skin = ThemeManager::instance().skin();
    const QString borderColor = skin.cvBorder.name();
    const QString bgColor = skin.cvBackground.name();
    const QString textColor = skin.cvText.name();
    const QString accentColor = skin.sbLogo.name();
    const QString indexColor = skin.cvIndex.name();

    const bool isDark = skin.cvBackground.lightness() < 128;
    QColor accent(skin.sbLogo);
    QColor accentHover = accent.lighter(isDark ? 115 : 110);
    QColor accentPressed = accent.darker(isDark ? 115 : 110);
    const QString accentHoverStr = accentHover.name();
    const QString accentPressedStr = accentPressed.name();
    // 下拉列表选中/悬停背景（与更多输入菜单一致）
    const QColor listHover = isDark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 35);
    const QString listHoverStr = QStringLiteral("rgba(%1, %2, %3, %4)")
        .arg(listHover.red()).arg(listHover.green()).arg(listHover.blue())
        .arg(listHover.alpha() / 255.0, 0, 'f', 2);

    if (hintLabel_) {
        hintLabel_->setStyleSheet(QStringLiteral(
            "QLabel { color: %1; font-size: 13px; letter-spacing: 0.3px; }").arg(indexColor));
    }
    // 编辑框 + 下拉箭头 + 弹出列表统一主题（弹出层用主题色边框，避免系统默认黑边）
    const QString comboStyle = QStringLiteral(
        "QComboBox {"
        "  min-width: 100px; padding: 6px 12px; padding-right: 24px;"
        "  border: 1px solid %1; border-radius: 6px;"
        "  background: %2; color: %3; font-size: 13px;"
        "}"
        "QComboBox:hover { border-color: %4; }"
        "QComboBox::drop-down {"
        "  subcontrol-origin: padding; subcontrol-position: right center;"
        "  width: 20px; border: none; border-left: 1px solid %1;"
        "  border-top-right-radius: 5px; border-bottom-right-radius: 5px;"
        "  background: transparent;"
        "}"
        "QComboBox::down-arrow { width: 12px; height: 12px; }"
        "QComboBox QFrame {"
        "  background: %2; border: 1px solid %1; border-radius: 6px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  outline: none; border: none; margin: 0;"
        "  background: %2; color: %3; font-size: 13px;"
        "  padding: 4px 0; min-width: 120px;"
        "  selection-background-color: %5; selection-color: %3;"
        "}"
        "QComboBox QAbstractItemView::item:focus { border: none; outline: none; }"
        "QComboBox QScrollBar:vertical { background: %2; border: none; border-radius: 4px; width: 8px; }"
        "QComboBox QScrollBar::handle:vertical { background: %1; border-radius: 4px; min-height: 24px; }"
        "QComboBox QScrollBar::add-line:vertical, QComboBox QScrollBar::sub-line:vertical { height: 0; }"
        "QComboBox QAbstractItemView::item { min-height: 28px; padding: 4px 12px; }"
        "QComboBox QAbstractItemView::item:hover { background: %5; }"
        ).arg(borderColor, bgColor, textColor, accentColor, listHoverStr);
    if (languageCombo_)
        languageCombo_->setStyleSheet(comboStyle);
    if (deviceCombo_)
        deviceCombo_->setStyleSheet(comboStyle);
    if (micButton_) {
        const QString baseStyle = QStringLiteral(
            "QPushButton {"
            "  background: %1; border: none; border-radius: 40px; color: white;"
            "}"
            "QPushButton:hover { background: %2; color: white; }"
            "QPushButton:pressed { background: %3; color: white; }");
        const QString recordingStyle = QStringLiteral(
            "QPushButton { background: %1; border: none; border-radius: 40px; color: white; }");
        micButton_->setStyleSheet(isRecording_
            ? recordingStyle.arg(accentPressedStr)
            : baseStyle.arg(accentColor, accentHoverStr, accentPressedStr));
    }
    update();
}

void VoiceInputDialog::updateMicButtonIcon() {
    if (!micButton_) return;
    if (svgIconsLoaded_) {
        micButton_->setText(QString());
        micButton_->setIcon(isRecording_ ? stopIcon_ : micIcon_);
    } else {
        micButton_->setIcon(QIcon());
        micButton_->setText(isRecording_
            ? QString::fromUtf8("\u25A0")  // 停止方块
            : QString::fromUtf8("\xF0\x9F\x8E\xA4"));  // 麦克风 emoji 后备
    }
}

void VoiceInputDialog::refreshAudioDevices() {
    deviceCombo_->clear();
    const auto devices = VoskSpeechEngine::availableAudioInputDevices();
    for (const auto &pair : devices) {
        deviceCombo_->addItem(pair.first, pair.second);
    }
    if (deviceCombo_->count() == 0) {
        deviceCombo_->addItem(QStringLiteral("麦克风(未检测到)"), QByteArray());
    }
}

void VoiceInputDialog::refreshAudioDevicesAsync() {
    using DeviceList = QList<QPair<QString, QByteArray>>;
    auto *watcher = new QFutureWatcher<DeviceList>(this);
    connect(watcher, &QFutureWatcher<DeviceList>::finished,
            this, [this, watcher]() {
        watcher->deleteLater();
        if (!deviceCombo_) return;
        deviceCombo_->clear();
        const auto devices = watcher->result();
        for (const auto &pair : devices) {
            deviceCombo_->addItem(pair.first, pair.second);
        }
        if (deviceCombo_->count() == 0) {
            deviceCombo_->addItem(QStringLiteral("麦克风(未检测到)"), QByteArray());
        }
    });
    watcher->setFuture(QtConcurrent::run(&VoskSpeechEngine::availableAudioInputDevices));
}

VoiceInputDialog::Language VoiceInputDialog::language() const {
    if (!languageCombo_) return Language::Mandarin;
    int v = languageCombo_->currentData().toInt();
    return static_cast<Language>(v);
}

QByteArray VoiceInputDialog::selectedDeviceId() const {
    if (!deviceCombo_) return {};
    return deviceCombo_->currentData().toByteArray();
}

void VoiceInputDialog::onMicButtonClicked() {
    isRecording_ = !isRecording_;
    if (micButton_) {
        updateSkinStyles();
        updateMicButtonIcon();
    }
    if (isRecording_) {
        emit startRecordingRequested();
    } else {
        emit stopRecordingRequested();
    }
}

void VoiceInputDialog::onLanguageChanged(int) {
    // 切换语言时需重新加载 Vosk 模型
}

void VoiceInputDialog::onDeviceChanged(int) {
    // 切换设备时需重新配置音频源
}

void VoiceInputDialog::setRecordingState(bool recording) {
    if (isRecording_ != recording) {
        isRecording_ = recording;
        if (micButton_) {
            updateSkinStyles();
            updateMicButtonIcon();
        }
        update();
    }
}

void VoiceInputDialog::onTextRecognized(const QString &text) {
    setRecordingState(false);
    emit textRecognized(text);
}

void VoiceInputDialog::popup(const QPoint &pos) {
    QScreen *screen = QApplication::screenAt(pos);
    if (!screen) screen = QApplication::primaryScreen();
    QRect sr = screen->availableGeometry();
    int x = pos.x() - width() / 2;
    int y = pos.y() - height() - 8;
    x = qBound(sr.left(), x, sr.right() - width());
    y = qBound(sr.top(), y, sr.bottom() - height());
    move(x, y);
    show();
    raise();
    activateWindow();
}

void VoiceInputDialog::showCentered(QWidget *parent) {
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

QRectF VoiceInputDialog::closeButtonRect() const {
    return QRectF(kShadowMargin + kDialogWidth - 36, kShadowMargin + 8, 28, 28);
}

QRectF VoiceInputDialog::micButtonRect() const {
    int cx = width() / 2;
    int cy = kShadowMargin + 120;
    return QRectF(cx - kMicButtonSize / 2.0, cy - kMicButtonSize / 2.0,
                  kMicButtonSize, kMicButtonSize);
}

void VoiceInputDialog::paintEvent(QPaintEvent *) {
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

    const auto &skin = ThemeManager::instance().skin();
    QPainterPath bgPath;
    bgPath.addRoundedRect(contentRect, kRadius, kRadius);
    p.fillPath(bgPath, skin.cvBackground);
    p.setPen(QPen(skin.cvBorder, 1));
    p.drawPath(bgPath);

    p.setClipPath(bgPath);

    // 顶部 Logo + 标题（与关于对话框风格一致）
    QRectF logoRect(kShadowMargin + 16, kShadowMargin + 12, 36, 36);
    QPainterPath logoPath;
    logoPath.addRoundedRect(logoRect, 8, 8);
    p.fillPath(logoPath, QColor(skin.sbLogo.red(), skin.sbLogo.green(),
                                skin.sbLogo.blue(), 30));
    p.setPen(skin.sbLogo);
    QFont logoFont;
    logoFont.setPointSize(20);
    logoFont.setWeight(QFont::Bold);
    p.setFont(logoFont);
    p.drawText(logoRect, Qt::AlignCenter, QStringLiteral("\u7075"));

    QFont titleFont;
    titleFont.setPointSize(15);
    titleFont.setWeight(QFont::Bold);
    p.setFont(titleFont);
    p.setPen(skin.cvText);
    p.drawText(QRectF(kShadowMargin + 58, kShadowMargin + 14,
                      kDialogWidth - 70, 32),
               Qt::AlignVCenter | Qt::AlignLeft,
               QStringLiteral("\u8bed\u97f3\u8f93\u5165"));

    // 关闭按钮
    QRectF closeRect = closeButtonRect();
    if (closeHovered_) {
        QPainterPath closePath;
        closePath.addRoundedRect(closeRect, 6, 6);
        p.fillPath(closePath, QColor(0, 0, 0, 25));
    }
    QFont closeFont;
    closeFont.setPointSize(16);
    p.setFont(closeFont);
    p.setPen(skin.cvIndex);
    p.drawText(closeRect, Qt::AlignCenter, QStringLiteral("\u00D7"));
}

void VoiceInputDialog::focusOutEvent(QFocusEvent *event) {
    QWidget::focusOutEvent(event);
    // 仅通过右上角 X 或 Escape 关闭，失去焦点时不自动关闭
}

void VoiceInputDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // 只有点击右上角 X 才关闭，点击其他区域（包括阴影区）不关闭
        if (closeButtonRect().contains(event->pos())) {
            close();
            emit closed();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void VoiceInputDialog::mouseMoveEvent(QMouseEvent *event) {
    bool hovered = closeButtonRect().contains(event->pos());
    if (hovered != closeHovered_) {
        closeHovered_ = hovered;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void VoiceInputDialog::showEvent(QShowEvent *event) {
    updateSkinStyles();
    QWidget::showEvent(event);
}

void VoiceInputDialog::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        close();
        emit closed();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}
