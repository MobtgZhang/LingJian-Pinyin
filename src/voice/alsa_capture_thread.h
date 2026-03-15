#pragma once

#include <QThread>
#include <QString>
#include <QByteArray>
#include <atomic>

#if LINGJIAN_HAVE_ALSA

/** ALSA 麦克风采集线程，轻量级替代 Qt Multimedia */
class AlsaCaptureThread : public QThread {
    Q_OBJECT
public:
    explicit AlsaCaptureThread(QObject* parent = nullptr) : QThread(parent) {}
    QString deviceId;
    int sampleRate = 16000;
    int chunkSamples = 1600;
    std::atomic<bool> stopFlag{false};

    void run() override;

signals:
    void chunkReady(const QByteArray& data);
    void openFailed(const QString& msg);
};

#endif
