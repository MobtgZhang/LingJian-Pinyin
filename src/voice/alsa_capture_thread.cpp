#include "alsa_capture_thread.h"

#if LINGJIAN_HAVE_ALSA
#include <alsa/asoundlib.h>
#include <QStringList>

void AlsaCaptureThread::run() {
    snd_pcm_t* handle = nullptr;
    QStringList toTry;
    if (!deviceId.isEmpty())
        toTry << deviceId;
    toTry << QStringLiteral("default")
          << QStringLiteral("pulse")
          << QStringLiteral("sysdefault")
          << QStringLiteral("plughw:0,0")
          << QStringLiteral("plughw:1,0")
          << QStringLiteral("hw:0,0")
          << QStringLiteral("hw:1,0");
    QString lastErr;
    for (const QString& devName : toTry) {
        if (stopFlag) return;
        QByteArray devBytes = devName.toUtf8();
        const char* dev = devBytes.constData();
        int err = snd_pcm_open(&handle, dev, SND_PCM_STREAM_CAPTURE, 0);
        if (err >= 0)
            break;
        lastErr = QString::fromUtf8(snd_strerror(err));
        if (handle) {
            snd_pcm_close(handle);
            handle = nullptr;
        }
    }
    if (!handle) {
        emit openFailed(lastErr.isEmpty() ? QStringLiteral("无法打开任何麦克风设备") : lastErr);
        return;
    }

    snd_pcm_hw_params_t* params = nullptr;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, 1);
    unsigned int sr = static_cast<unsigned int>(sampleRate);
    snd_pcm_hw_params_set_rate_near(handle, params, &sr, nullptr);
    snd_pcm_uframes_t frames = static_cast<snd_pcm_uframes_t>(chunkSamples);
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, nullptr);
    int err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        snd_pcm_close(handle);
        emit openFailed(QString::fromUtf8(snd_strerror(err)));
        return;
    }

    QByteArray buf(static_cast<int>(chunkSamples * 2), 0);
    while (!stopFlag && handle) {
        int n = snd_pcm_readi(handle, buf.data(), chunkSamples);
        if (n == -EPIPE) {
            snd_pcm_prepare(handle);
            continue;
        }
        if (n == -ESTRPIPE || n < 0) break;
        if (n > 0) {
            QByteArray chunk = buf.left(n * 2);
            emit chunkReady(chunk);
        }
    }
    snd_pcm_close(handle);
}

#endif
