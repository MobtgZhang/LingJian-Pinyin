/**
 * 语音输入测试程序
 * 打开麦克风后，若收到语音识别结果（你说了话并被识别），则显示「测试成功」。
 */
#include "vosk_speech_engine.h"

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QGroupBox>

class VoiceTestWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit VoiceTestWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        auto *central = new QWidget(this);
        setCentralWidget(central);
        auto *layout = new QVBoxLayout(central);
        layout->setSpacing(12);

        setWindowTitle(QStringLiteral("语音输入测试"));
        setMinimumSize(420, 320);

        // 状态
        statusLabel_ = new QLabel(QStringLiteral("点击「开始测试」打开麦克风，然后对着麦克风说话。"));
        statusLabel_->setWordWrap(true);
        statusLabel_->setStyleSheet(QStringLiteral("QLabel { font-size: 13px; }"));
        layout->addWidget(statusLabel_);

        // 识别结果展示
        resultEdit_ = new QPlainTextEdit(this);
        resultEdit_->setReadOnly(true);
        resultEdit_->setPlaceholderText(QStringLiteral("识别到的文字会显示在这里…"));
        resultEdit_->setMaximumBlockCount(500);
        layout->addWidget(resultEdit_, 1);

        // 语言与设备
        auto *optionsBox = new QGroupBox(QStringLiteral("选项"));
        auto *optionsLayout = new QHBoxLayout(optionsBox);
        optionsLayout->addWidget(new QLabel(QStringLiteral("语言:")));
        languageCombo_ = new QComboBox(this);
        languageCombo_->addItem(QStringLiteral("普通话"), static_cast<int>(VoskSpeechEngine::Language::Mandarin));
        languageCombo_->addItem(QStringLiteral("英语"), static_cast<int>(VoskSpeechEngine::Language::English));
        optionsLayout->addWidget(languageCombo_);
        optionsLayout->addWidget(new QLabel(QStringLiteral("麦克风:")));
        deviceCombo_ = new QComboBox(this);
        const auto devices = VoskSpeechEngine::availableAudioInputDevices();
        for (const auto &pair : devices) {
            deviceCombo_->addItem(pair.first, pair.second);
        }
        if (deviceCombo_->count() == 0) {
            deviceCombo_->addItem(QStringLiteral("(未检测到)"), QByteArray());
        }
        optionsLayout->addWidget(deviceCombo_, 1);
        layout->addWidget(optionsBox);

        // 按钮
        startBtn_ = new QPushButton(QStringLiteral("开始测试（打开麦克风）"), this);
        stopBtn_ = new QPushButton(QStringLiteral("停止"), this);
        stopBtn_->setEnabled(false);

        auto *btnLayout = new QHBoxLayout;
        btnLayout->addWidget(startBtn_);
        btnLayout->addWidget(stopBtn_);
        btnLayout->addStretch();
        layout->addLayout(btnLayout);

        engine_ = new VoskSpeechEngine(this);

        connect(startBtn_, &QPushButton::clicked, this, &VoiceTestWindow::onStart);
        connect(stopBtn_, &QPushButton::clicked, this, &VoiceTestWindow::onStop);
        connect(engine_, &VoskSpeechEngine::partialResult, this, &VoiceTestWindow::onPartialResult);
        connect(engine_, &VoskSpeechEngine::finalResult, this, &VoiceTestWindow::onFinalResult);
        connect(engine_, &VoskSpeechEngine::errorOccurred, this, &VoiceTestWindow::onError);
    }

private slots:
    void onStart() {
        if (!engine_->isAvailable()) {
            QMessageBox::warning(this, QStringLiteral("语音测试"),
                QStringLiteral("语音识别不可用，请安装 libvosk 及相应模型。"));
            return;
        }
        engine_->setLanguage(static_cast<VoskSpeechEngine::Language>(
            languageCombo_->currentData().toInt()));
        engine_->setAudioDevice(deviceCombo_->currentData().toByteArray());

        resultEdit_->clear();
        testSuccess_ = false;
        if (!engine_->start()) {
            statusLabel_->setText(QStringLiteral("打开麦克风失败，请查看错误提示。"));
            return;
        }
        statusLabel_->setText(QStringLiteral("麦克风已打开，请对着麦克风说话… 识别到内容即表示测试成功。"));
        startBtn_->setEnabled(false);
        stopBtn_->setEnabled(true);
    }

    void onStop() {
        if (engine_->isRecording()) {
            engine_->stop();
        }
        startBtn_->setEnabled(true);
        stopBtn_->setEnabled(false);
        if (testSuccess_) {
            statusLabel_->setText(QStringLiteral("测试成功：已打开麦克风并识别到你的声音。"));
        } else {
            statusLabel_->setText(QStringLiteral("已停止。若未识别到任何内容，可再次点击「开始测试」重试。"));
        }
    }

    void onPartialResult(const QString &text) {
        if (text.isEmpty()) return;
        testSuccess_ = true;
        resultEdit_->appendPlainText(QStringLiteral("[实时] ") + text);
    }

    void onFinalResult(const QString &text) {
        if (text.isEmpty()) return;
        testSuccess_ = true;
        resultEdit_->appendPlainText(QStringLiteral("[最终] ") + text);
        statusLabel_->setText(QStringLiteral("测试成功：已识别到你的输入。"));
    }

    void onError(const QString &message) {
        QMessageBox::warning(this, QStringLiteral("语音测试"), message);
        onStop();
    }

private:
    QLabel *statusLabel_ = nullptr;
    QPlainTextEdit *resultEdit_ = nullptr;
    QComboBox *languageCombo_ = nullptr;
    QComboBox *deviceCombo_ = nullptr;
    QPushButton *startBtn_ = nullptr;
    QPushButton *stopBtn_ = nullptr;
    VoskSpeechEngine *engine_ = nullptr;
    bool testSuccess_ = false;
};

#include "voice_input_test.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    VoiceTestWindow w;
    w.show();
    return app.exec();
}
