#pragma once

#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QString>

struct SkinConfig {
    QString name;
    QString author;
    QString version;

    // Display
    int candidateFontSize = 14;
    int preeditFontSize = 13;
    int indexFontSize = 12;
    int statusFontSize = 13;

    // CandidateView
    QColor cvBackground{255, 255, 255, 245};
    QColor cvBorder{210, 210, 210};
    QColor cvText{51, 51, 51};
    QColor cvHighlight{74, 144, 217};
    QColor cvPreedit{255, 102, 0};
    QColor cvIndex{153, 153, 153};
    QColor cvSeparator{232, 232, 232};
    QPixmap cvBackgroundImage;
    int cvBorderRadius = 8;
    int cvPadding = 8;

    // StatusBar
    QColor sbBackground{255, 255, 255, 245};
    QColor sbBorder{210, 210, 210};
    QColor sbText{50, 50, 50};
    QColor sbHover{0, 0, 0, 18};
    QColor sbLogo{255, 120, 0};
    QColor sbAi{40, 130, 220};
    QPixmap sbBackgroundImage;
    int sbBorderRadius = 6;
};

class SkinLoader {
public:
    SkinLoader() = default;

    bool loadFromDirectory(const QString &dirPath);

    bool loadFromZip(const QString &zipPath);

    const SkinConfig &config() const { return config_; }

    static SkinConfig darkSkin();
    static SkinConfig lightSkin();
    static SkinConfig blueSkin();
    static SkinConfig greenSkin();
    static SkinConfig purpleSkin();
    static SkinConfig roseSkin();
    static SkinConfig sunsetSkin();
    static SkinConfig iceSkin();

private:
    bool parseIniFile(const QString &iniPath, const QString &baseDir);
    static QColor parseColor(const QString &str);

    SkinConfig config_;
    QString tempDir_;
};
