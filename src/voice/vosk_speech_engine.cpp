#include "vosk_speech_engine.h"

#if LINGJIAN_HAVE_ALSA
#include "alsa_capture_thread.h"
#include <alsa/asoundlib.h>
#include <alsa/error.h>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#endif

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>
#include <QFutureWatcher>

#if LINGJIAN_HAVE_QT_MULTIMEDIA
#include <QMediaDevices>
#include <QAudioSource>
#include <QAudioDevice>
#include <QAudioFormat>
#endif

#ifdef __linux__
#include <dlfcn.h>
#endif

// Vosk C API 函数指针类型
typedef void* (*vosk_model_new_t)(const char*);
typedef void (*vosk_model_free_t)(void*);
typedef void* (*vosk_recognizer_new_t)(void*, float);
typedef int (*vosk_recognizer_accept_waveform_s_t)(void*, const short*, int);
typedef const char* (*vosk_recognizer_result_t)(void*);
typedef const char* (*vosk_recognizer_partial_result_t)(void*);
typedef const char* (*vosk_recognizer_final_result_t)(void*);
typedef void (*vosk_recognizer_reset_t)(void*);
typedef void (*vosk_recognizer_free_t)(void*);
typedef void (*vosk_set_log_level_t)(int);

struct VoskApi {
    void* lib = nullptr;
    vosk_model_new_t model_new = nullptr;
    vosk_model_free_t model_free = nullptr;
    vosk_recognizer_new_t recognizer_new = nullptr;
    vosk_recognizer_accept_waveform_s_t recognizer_accept_waveform_s = nullptr;
    vosk_recognizer_result_t recognizer_result = nullptr;
    vosk_recognizer_partial_result_t recognizer_partial_result = nullptr;
    vosk_recognizer_final_result_t recognizer_final_result = nullptr;
    vosk_recognizer_reset_t recognizer_reset = nullptr;
    vosk_recognizer_free_t recognizer_free = nullptr;
    vosk_set_log_level_t set_log_level = nullptr;
};

class VoskSpeechEngine::Impl {
public:
    VoskApi api;
    void* model = nullptr;
    void* recognizer = nullptr;
    VoskSpeechEngine::Language language = VoskSpeechEngine::Language::Mandarin;
    QByteArray selectedDeviceId_;
    QString loadError_;  // loadVosk 失败时的原因（dlerror 等）

#if LINGJIAN_HAVE_QT_MULTIMEDIA
    QAudioSource* audioSource = nullptr;
    QIODevice* audioDevice = nullptr;
#endif

#if LINGJIAN_HAVE_ALSA
    AlsaCaptureThread* alsaThread = nullptr;
#endif

    static constexpr int kSampleRate = 16000;
    static constexpr int kChunkMs = 100;
    static constexpr int kChunkSamples = kSampleRate * kChunkMs / 1000;

    bool loadVosk() {
#ifdef __linux__
        auto tryOpen = [this](const QString& path) -> bool {
            QByteArray pathBytes = path.toUtf8();
            api.lib = dlopen(pathBytes.constData(), RTLD_NOW);
            return api.lib != nullptr;
        };

        const char* libNames[] = {"libvosk.so", "libvosk.so.0"};
        // 1) 系统路径（LD_LIBRARY_PATH、/usr/lib 等）
        for (const char* name : libNames) {
            if (tryOpen(QString::fromUtf8(name))) break;
        }
        // 2) 环境变量 VOSK_LIB_PATH 指定目录（可指向 libvosk 所在目录或 .so 完整路径）
        if (!api.lib) {
            QByteArray envPath = qgetenv("VOSK_LIB_PATH");
            if (!envPath.isEmpty()) {
                QString path = QString::fromUtf8(envPath).trimmed();
                if (path.endsWith(".so")) {
                    if (tryOpen(path)) { /* ok */ }
                } else {
                    QDir dir(path);
                    for (const char* name : libNames) {
                        if (tryOpen(dir.absoluteFilePath(QString::fromUtf8(name)))) break;
                    }
                }
            }
        }
        // 3) 程序目录及 ../lib（便于自带或安装到 prefix 的 lib）
        if (!api.lib) {
            QString appDir = QCoreApplication::applicationDirPath();
            QStringList searchDirs = {appDir, appDir + "/../lib", appDir + "/lib"};
            for (const QString& dirPath : searchDirs) {
                QDir dir(dirPath);
                for (const char* name : libNames) {
                    if (tryOpen(dir.absoluteFilePath(QString::fromUtf8(name)))) break;
                }
                if (api.lib) break;
            }
        }
        // 4) /usr/local/lib（常见安装位置）
        if (!api.lib) {
            for (const char* name : libNames) {
                if (tryOpen(QStringLiteral("/usr/local/lib/") + QString::fromUtf8(name))) break;
            }
        }

        if (!api.lib) {
            const char* err = dlerror();
            loadError_ = err ? QString::fromUtf8(err) : QStringLiteral("未知错误");
            qWarning("Vosk: 无法加载 libvosk（%s）。可设置环境变量 VOSK_LIB_PATH 指向 libvosk.so 所在目录。",
                     err ? err : "未知错误");
            return false;
        }

#define LOAD(name, type) do { \
    api.name = (type)dlsym(api.lib, "vosk_" #name); \
    if (!api.name) { \
        const char* err = dlerror(); \
        loadError_ = err ? QString::fromUtf8(err) : QStringLiteral("符号 vosk_" #name " 未找到"); \
        qWarning("Vosk: 解析符号 vosk_" #name " 失败: %s", err ? err : "未知"); \
        dlclose(api.lib); \
        api.lib = nullptr; \
        return false; \
    } \
} while(0)

        LOAD(model_new, vosk_model_new_t);
        LOAD(model_free, vosk_model_free_t);
        LOAD(recognizer_new, vosk_recognizer_new_t);
        LOAD(recognizer_accept_waveform_s, vosk_recognizer_accept_waveform_s_t);
        LOAD(recognizer_result, vosk_recognizer_result_t);
        LOAD(recognizer_partial_result, vosk_recognizer_partial_result_t);
        LOAD(recognizer_final_result, vosk_recognizer_final_result_t);
        LOAD(recognizer_reset, vosk_recognizer_reset_t);
        LOAD(recognizer_free, vosk_recognizer_free_t);
        LOAD(set_log_level, vosk_set_log_level_t);
#undef LOAD
        return true;
#else
        (void)api;
        return false;
#endif
    }

    QString findModelPath(VoskSpeechEngine::Language lang) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString curDir = QDir::currentPath();
        QStringList basePaths = {
            appDir + "/../../../data/vosk-models",
            appDir + "/../../data/vosk-models",
            appDir + "/../data/vosk-models",
            appDir + "/data/vosk-models",
            curDir + "/data/vosk-models",
            curDir + "/../data/vosk-models",
            "data/vosk-models",
            "../data/vosk-models",
            QDir::homePath() + "/.local/share/lingjian/vosk-models",
        };
        QString modelDir = (lang == VoskSpeechEngine::Language::Mandarin) ? "vosk-model-small-cn-0.22" : "vosk-model-small-en-us-0.15";
        for (const QString& base : basePaths) {
            QString path = base + "/" + modelDir;
            if (QDir(path).exists()) return path;
        }
        return {};
    }

    static QString parseJsonText(const char* json, const char* key = "text") {
        if (!json) return {};
        QString s = QString::fromUtf8(json);
        QString keyStr = QString("\"%1\"").arg(key);
        int start = s.indexOf(keyStr);
        if (start < 0) return {};
        start = s.indexOf(':', start) + 1;
        int end = s.indexOf('"', start + 1);
        if (end <= start) return {};
        return s.mid(start + 1, end - start - 1).trimmed();
    }
};

// 在后台线程中加载模型与识别器，避免阻塞主线程
struct ModelLoadResult {
    void* model = nullptr;
    void* recognizer = nullptr;
    QString error;
};
static ModelLoadResult loadModelInBackground(const QString& modelPath, int sampleRate, const VoskApi& api) {
    ModelLoadResult r;
    r.model = api.model_new(modelPath.toUtf8().constData());
    if (!r.model) {
        r.error = QStringLiteral("加载语音模型失败");
        return r;
    }
    r.recognizer = api.recognizer_new(r.model, static_cast<float>(sampleRate));
    if (!r.recognizer) {
        if (api.model_free) api.model_free(r.model);
        r.model = nullptr;
        r.error = QStringLiteral("创建识别器失败");
        return r;
    }
    return r;
}

#if LINGJIAN_HAVE_ALSA
// 过滤 ALSA 在探测设备时产生的无害提示（PulseAudio/JACK 未运行、配置解析、不存在的声卡等）
static void alsaFilteredErrorHandler(const char* file, int line, const char* func, int err,
                                     const char* fmt, ...) {
    char buf[384];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    (void)err;
    const char* ignore[] = {
        /* JACK / PulseAudio 未运行 */
        "Cannot connect to server socket",
        "Cannot connect to server request channel",
        "jack server is not running",
        "JackShmReadWritePtr",
        "Init not done for -1",
        /* OSS */
        "pcm_oss",
        "/dev/dsp",
        /* ALSA 配置解析时对不存在的声卡（PCH/NVidia 等）的探测失败 */
        "parse_card",
        "cannot find card",
        "Unknown parameter",
        "snd_config_expand",
        "snd_config_evaluate",
        "Parse arguments error",
        "No such device",
        "snd_func_refer",
        "Unable to find definition",
        "Parameter CARD must be an integer",
        "Invalid argument",
        "Evaluate error",
        "error evaluating",
        "No such file or directory",
    };
    for (const char* sub : ignore) {
        if (strstr(buf, sub) != nullptr)
            return;
    }
    if (file && func)
        fprintf(stderr, "ALSA %s:%d %s: %s\n", file, line, func, buf);
    else
        fprintf(stderr, "ALSA: %s\n", buf);
}
#endif

#if LINGJIAN_HAVE_ALSA
// 在首次使用 ALSA 前注册过滤型错误处理器，避免控制台被配置探测等无害信息刷屏
static void ensureAlsaErrorHandlerSet() {
    static bool set = false;
    if (!set) {
        snd_lib_error_set_handler(alsaFilteredErrorHandler);
        set = true;
    }
}
#endif

QList<QPair<QString, QByteArray>> VoskSpeechEngine::availableAudioInputDevices() {
    QList<QPair<QString, QByteArray>> list;

#if LINGJIAN_HAVE_ALSA
    ensureAlsaErrorHandlerSet();
    // 只枚举麦克风（输入）设备，不加入“系统默认”，避免加载所有驱动
    void** hints = nullptr;
    if (snd_device_name_hint(-1, "pcm", &hints) >= 0 && hints) {
        for (void** p = hints; *p; ++p) {
            char* name = snd_device_name_get_hint(*p, "NAME");
            char* desc = snd_device_name_get_hint(*p, "DESC");
            char* ioid = snd_device_name_get_hint(*p, "IOID");
            if (!name) continue;
            // 仅保留输入设备（排除纯输出），只加载麦克风
            if (ioid && strcmp(ioid, "Output") == 0) {
                if (name) free(name);
                if (desc) free(desc);
                if (ioid) free(ioid);
                continue;
            }
            // 验证该设备是否支持采集（避免列出仅播放设备）
            snd_pcm_t* test = nullptr;
            if (snd_pcm_open(&test, name, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK) == 0) {
                snd_pcm_close(test);
                QString descStr = desc ? QString::fromUtf8(desc).split('\n').first() : QString::fromUtf8(name);
                list.append(qMakePair(descStr, QByteArray(name)));
            }
            if (name) free(name);
            if (desc) free(desc);
            if (ioid) free(ioid);
        }
        snd_device_name_free_hint(hints);
    }
    if (list.isEmpty()) {
        list.append(qMakePair(QStringLiteral("麦克风(未检测到)"), QByteArray()));
    }
#elif LINGJIAN_HAVE_QT_MULTIMEDIA
    // 只列麦克风（输入）设备，不加入“系统默认”
    const auto inputs = QMediaDevices::audioInputs();
    for (const QAudioDevice& dev : inputs) {
        QString desc = dev.description();
        if (desc.isEmpty()) desc = QStringLiteral("麦克风");
        list.append(qMakePair(desc, dev.id()));
    }
    if (list.isEmpty()) {
        list.append(qMakePair(QStringLiteral("麦克风(未检测到)"), QByteArray()));
    }
#else
    list.append(qMakePair(QStringLiteral("麦克风(需要 ALSA 或 Qt Multimedia)"), QByteArray()));
#endif
    return list;
}

VoskSpeechEngine::VoskSpeechEngine(QObject* parent)
    : QObject(parent) {
    impl_ = new Impl;
#if LINGJIAN_HAVE_ALSA
    ensureAlsaErrorHandlerSet();
#endif
    available_ = impl_->loadVosk();
    if (available_ && impl_->api.set_log_level) {
        impl_->api.set_log_level(-1);
    }
}

VoskSpeechEngine::~VoskSpeechEngine() {
    stop();
    if (impl_->recognizer && impl_->api.recognizer_free) {
        impl_->api.recognizer_free(impl_->recognizer);
        impl_->recognizer = nullptr;
    }
    if (impl_->model && impl_->api.model_free) {
        impl_->api.model_free(impl_->model);
        impl_->model = nullptr;
    }
#ifdef __linux__
    if (impl_->api.lib) dlclose(impl_->api.lib);
#endif
    delete impl_;
}

void VoskSpeechEngine::setLanguage(Language lang) {
    impl_->language = lang;
}

void VoskSpeechEngine::setAudioDevice(const QByteArray& deviceId) {
    impl_->selectedDeviceId_ = deviceId;
}

void VoskSpeechEngine::feedAudioChunk(const QByteArray& data) {
    if (!impl_->recognizer || data.size() < 2) return;
    int nsamples = data.size() / 2;
    if (nsamples > impl_->kChunkSamples * 2) nsamples = impl_->kChunkSamples * 2;
    int r = impl_->api.recognizer_accept_waveform_s(impl_->recognizer,
        reinterpret_cast<const short*>(data.constData()), nsamples);
    if (r == 1) {
        const char* res = impl_->api.recognizer_result(impl_->recognizer);
        QString text = Impl::parseJsonText(res);
        if (!text.isEmpty()) emit finalResult(text);
        impl_->api.recognizer_reset(impl_->recognizer);
    } else if (impl_->api.recognizer_partial_result) {
        const char* partial = impl_->api.recognizer_partial_result(impl_->recognizer);
        QString text = Impl::parseJsonText(partial, "partial");
        if (!text.isEmpty()) emit partialResult(text);
    }
}

bool VoskSpeechEngine::start() {
    if (recording_) return true;
    if (starting_) return true;  // 已在异步加载中，避免重复
    if (!available_) {
        QString msg = QStringLiteral("语音识别不可用，请安装 libvosk 及相应模型。");
        emit errorOccurred(msg);
        return false;
    }

    QString modelPath = impl_->findModelPath(impl_->language);
    if (modelPath.isEmpty()) {
        emit errorOccurred(QStringLiteral("未找到语音模型，请将模型放置于 data/vosk-models/ 或运行 run.sh 自动下载"));
        return false;
    }

    if (impl_->model) {
        impl_->api.model_free(impl_->model);
        impl_->model = nullptr;
    }
    if (impl_->recognizer) {
        impl_->api.recognizer_free(impl_->recognizer);
        impl_->recognizer = nullptr;
    }

    // 在后台线程加载模型，避免阻塞主线程导致界面卡死
    starting_ = true;
    recording_ = true;  // 标记“正在启动”，若用户在此期间点停止则 onModelLoadFinished 中会跳过
    const QString path = modelPath;
    const int rate = impl_->kSampleRate;
    const VoskApi api = impl_->api;
    QFutureWatcher<ModelLoadResult>* watcher = new QFutureWatcher<ModelLoadResult>(this);
    connect(watcher, &QFutureWatcher<ModelLoadResult>::finished, this, [this, watcher]() {
        watcher->deleteLater();
        starting_ = false;
        ModelLoadResult r = watcher->result();
        if (!recording_) {
            // 用户已在加载期间点击停止，释放已加载的模型并丢弃结果
            if (r.recognizer && impl_->api.recognizer_free) impl_->api.recognizer_free(r.recognizer);
            if (r.model && impl_->api.model_free) impl_->api.model_free(r.model);
            return;
        }
        if (!r.model || !r.recognizer) {
            recording_ = false;
            if (!r.error.isEmpty()) emit errorOccurred(r.error);
            emit startFinished(false);
            return;
        }
        impl_->model = r.model;
        impl_->recognizer = r.recognizer;

#if LINGJIAN_HAVE_ALSA
        impl_->alsaThread = new AlsaCaptureThread(this);
        impl_->alsaThread->deviceId = QString::fromUtf8(impl_->selectedDeviceId_.isEmpty() ? "default" : impl_->selectedDeviceId_.constData());
        impl_->alsaThread->sampleRate = impl_->kSampleRate;
        impl_->alsaThread->chunkSamples = impl_->kChunkSamples;
        connect(impl_->alsaThread, &AlsaCaptureThread::chunkReady, this, [this](const QByteArray& data) {
            if (recording_ && impl_->recognizer)
                feedAudioChunk(data);
        });
        connect(impl_->alsaThread, &AlsaCaptureThread::openFailed, this, [this](const QString& msg) {
            recording_ = false;
            impl_->alsaThread = nullptr;
            emit errorOccurred(QStringLiteral("麦克风打开失败: ") + msg);
            emit startFinished(false);
        });
        impl_->alsaThread->start();
        recording_ = true;
        emit startFinished(true);
        return;
#endif

#if LINGJIAN_HAVE_QT_MULTIMEDIA
        QAudioFormat format;
        format.setSampleRate(impl_->kSampleRate);
        format.setChannelCount(1);
        format.setSampleFormat(QAudioFormat::Int16);

        QAudioDevice device;
        const auto inputs = QMediaDevices::audioInputs();
        if (!impl_->selectedDeviceId_.isEmpty()) {
            for (const QAudioDevice& d : inputs) {
                if (d.id() == impl_->selectedDeviceId_ && d.isFormatSupported(format)) {
                    device = d;
                    break;
                }
            }
        }
        if (device.isNull()) {
            for (const QAudioDevice& d : inputs) {
                if (d.isFormatSupported(format)) {
                    device = d;
                    break;
                }
            }
        }
        if (device.isNull()) device = QMediaDevices::defaultAudioInput();
        if (device.isNull()) {
            emit errorOccurred(QStringLiteral("未找到可用的麦克风设备"));
            impl_->api.recognizer_free(impl_->recognizer);
            impl_->recognizer = nullptr;
            emit startFinished(false);
            return;
        }

        if (impl_->audioSource) {
            delete impl_->audioSource;
            impl_->audioSource = nullptr;
        }
        impl_->audioSource = new QAudioSource(device, format, this);
        impl_->audioDevice = impl_->audioSource->start();

        if (!impl_->audioDevice) {
            emit errorOccurred(QStringLiteral("无法启动麦克风"));
            impl_->api.recognizer_free(impl_->recognizer);
            impl_->recognizer = nullptr;
            emit startFinished(false);
            return;
        }

        connect(impl_->audioDevice, &QIODevice::readyRead, this, [this]() {
            if (!recording_ || !impl_->audioDevice || !impl_->recognizer) return;
            QByteArray data = impl_->audioDevice->readAll();
            if (data.size() >= 2) feedAudioChunk(data);
        });
        recording_ = true;
        emit startFinished(true);
        return;
#endif

#if !LINGJIAN_HAVE_ALSA && !LINGJIAN_HAVE_QT_MULTIMEDIA
        emit errorOccurred(QStringLiteral("需要 ALSA 或 Qt6 Multimedia 支持麦克风，请安装 libasound2-dev 或 qt6-multimedia-dev"));
        impl_->api.recognizer_free(impl_->recognizer);
        impl_->recognizer = nullptr;
        emit startFinished(false);
#endif
    });
    watcher->setFuture(QtConcurrent::run(loadModelInBackground, path, rate, api));
    return true;
}

void VoskSpeechEngine::stop() {
    if (!recording_) return;
    recording_ = false;

#if LINGJIAN_HAVE_ALSA
    if (impl_->alsaThread) {
        impl_->alsaThread->stopFlag = true;
        impl_->alsaThread->wait(300);
        impl_->alsaThread = nullptr;
    }
#endif

#if LINGJIAN_HAVE_QT_MULTIMEDIA
    if (impl_->audioSource) {
        impl_->audioSource->stop();
        delete impl_->audioSource;
        impl_->audioSource = nullptr;
    }
    impl_->audioDevice = nullptr;
#endif

    if (impl_->recognizer && impl_->api.recognizer_final_result) {
        const char* res = impl_->api.recognizer_final_result(impl_->recognizer);
        QString text = Impl::parseJsonText(res);
        if (!text.isEmpty()) emit finalResult(text);
    }
}
