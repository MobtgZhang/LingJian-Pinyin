#pragma once

#include <QIcon>
#include <QShowEvent>
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>

class VoiceInputDialog : public QWidget {
    Q_OBJECT
public:
    enum class Language { Mandarin, English };

    explicit VoiceInputDialog(QWidget *parent = nullptr);

    void popup(const QPoint &pos);
    void showCentered(QWidget *parent);

    Language language() const;
    QByteArray selectedDeviceId() const;

    bool isRecording() const { return isRecording_; }
    void setRecordingState(bool recording);

public slots:
    void onTextRecognized(const QString &text);

signals:
    void startRecordingRequested();
    void stopRecordingRequested();
    void textRecognized(const QString &text);
    void closed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUi();
    void updateSkinStyles();
    void refreshAudioDevices();
    void refreshAudioDevicesAsync();
    void onMicButtonClicked();
    void onLanguageChanged(int index);
    void onDeviceChanged(int index);
    QRectF closeButtonRect() const;
    QRectF micButtonRect() const;
    QString findSvgPath(const QString &fileName) const;
    void updateMicButtonIcon();

    QLabel *hintLabel_ = nullptr;
    QComboBox *languageCombo_ = nullptr;
    QComboBox *deviceCombo_ = nullptr;
    QPushButton *micButton_ = nullptr;
    QShortcut *f2Shortcut_ = nullptr;

    bool closeHovered_ = false;
    bool micHovered_ = false;
    bool isRecording_ = false;

    QIcon micIcon_;
    QIcon stopIcon_;
    bool svgIconsLoaded_ = false;

    static constexpr int kDialogWidth = 380;
    static constexpr int kDialogHeight = 320;
    static constexpr int kRadius = 12;
    static constexpr int kShadowMargin = 8;
    static constexpr int kMicButtonSize = 80;
};
