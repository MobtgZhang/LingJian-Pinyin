#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QPair>

/** Vosk 离线语音识别引擎封装
 * 使用 https://github.com/alphacep/vosk-api 进行语音转文字
 * 支持普通话和英语模型。
 * 麦克风采集优先使用 ALSA（Linux 轻量），可选 Qt6 Multimedia。
 */
class VoskSpeechEngine : public QObject {
    Q_OBJECT
public:
    enum class Language { Mandarin, English };

    explicit VoskSpeechEngine(QObject *parent = nullptr);
    ~VoskSpeechEngine();

    /** 是否已加载 Vosk 库 */
    bool isAvailable() const { return available_; }

    /** 枚举可用的音频输入设备：(显示名称, 设备 ID) */
    static QList<QPair<QString, QByteArray>> availableAudioInputDevices();

    /** 设置语言（需在 start 前调用） */
    void setLanguage(Language lang);

    /** 设置音频输入设备 ID（空则使用默认） */
    void setAudioDevice(const QByteArray &deviceId);

    /** 开始录音识别（异步加载模型，完成后发出 startFinished） */
    bool start();

    /** 停止录音并返回最终识别结果 */
    void stop();

    /** 是否正在录音 */
    bool isRecording() const { return recording_; }

signals:
    /** 异步 start() 完成：true 表示已开始录音，false 表示失败（会先发出 errorOccurred） */
    void startFinished(bool success);
    void partialResult(const QString &text);
    void finalResult(const QString &text);
    void errorOccurred(const QString &message);

private:
    void feedAudioChunk(const QByteArray& data);
    class Impl;
    Impl *impl_ = nullptr;
    bool available_ = false;
    bool recording_ = false;
    bool starting_ = false;  // 异步加载模型进行中，防止重复 start
};
