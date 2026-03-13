#include "skin_loader.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUuid>

bool SkinLoader::loadFromDirectory(const QString &dirPath) {
    QString iniPath = QDir(dirPath).filePath(QStringLiteral("skin.ini"));
    if (!QFile::exists(iniPath)) {
        return false;
    }
    return parseIniFile(iniPath, dirPath);
}

bool SkinLoader::loadFromZip(const QString &zipPath) {
    if (!QFile::exists(zipPath)) {
        return false;
    }

    QString tempBase = QStandardPaths::writableLocation(
        QStandardPaths::TempLocation);
    QString extractDir = QDir(tempBase).filePath(
        QStringLiteral("lingjian_skin_") + QUuid::createUuid().toString(QUuid::Id128));

    QDir().mkpath(extractDir);

    QProcess proc;
    proc.setWorkingDirectory(extractDir);
    proc.start(QStringLiteral("unzip"), {QStringLiteral("-o"), zipPath});
    proc.waitForFinished(10000);

    if (proc.exitCode() != 0) {
        QDir(extractDir).removeRecursively();
        return false;
    }

    QString iniPath = QDir(extractDir).filePath(QStringLiteral("skin.ini"));
    if (!QFile::exists(iniPath)) {
        QStringList subdirs = QDir(extractDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &sub : subdirs) {
            QString subIni = QDir(extractDir).filePath(sub + QStringLiteral("/skin.ini"));
            if (QFile::exists(subIni)) {
                iniPath = subIni;
                extractDir = QDir(extractDir).filePath(sub);
                break;
            }
        }
    }

    if (!QFile::exists(iniPath)) {
        QDir(extractDir).removeRecursively();
        return false;
    }

    bool ok = parseIniFile(iniPath, extractDir);
    tempDir_ = extractDir;
    return ok;
}

bool SkinLoader::parseIniFile(const QString &iniPath, const QString &baseDir) {
    QSettings ini(iniPath, QSettings::IniFormat);

    ini.beginGroup(QStringLiteral("General"));
    config_.name = ini.value(QStringLiteral("name"), QStringLiteral("Default")).toString();
    config_.author = ini.value(QStringLiteral("author"), QStringLiteral("LingJian")).toString();
    config_.version = ini.value(QStringLiteral("version"), QStringLiteral("1.0")).toString();
    ini.endGroup();

    ini.beginGroup(QStringLiteral("Display"));
    config_.candidateFontSize = ini.value(QStringLiteral("candidate_font_size"), 14).toInt();
    config_.preeditFontSize = ini.value(QStringLiteral("preedit_font_size"), 13).toInt();
    config_.indexFontSize = ini.value(QStringLiteral("index_font_size"), 12).toInt();
    config_.statusFontSize = ini.value(QStringLiteral("status_font_size"), 13).toInt();
    ini.endGroup();

    ini.beginGroup(QStringLiteral("CandidateView"));
    config_.cvBackground = parseColor(ini.value(QStringLiteral("background_color"), "#FFFFFF").toString());
    config_.cvBorder = parseColor(ini.value(QStringLiteral("border_color"), "#D2D2D2").toString());
    config_.cvText = parseColor(ini.value(QStringLiteral("text_color"), "#333333").toString());
    config_.cvHighlight = parseColor(ini.value(QStringLiteral("highlight_color"), "#4A90D9").toString());
    config_.cvPreedit = parseColor(ini.value(QStringLiteral("preedit_color"), "#FF6600").toString());
    config_.cvIndex = parseColor(ini.value(QStringLiteral("index_color"), "#999999").toString());
    config_.cvSeparator = parseColor(ini.value(QStringLiteral("separator_color"), "#E8E8E8").toString());
    config_.cvBorderRadius = ini.value(QStringLiteral("border_radius"), 8).toInt();
    config_.cvPadding = ini.value(QStringLiteral("padding"), 8).toInt();

    QString bgImg = ini.value(QStringLiteral("background_image")).toString();
    if (!bgImg.isEmpty()) {
        QString imgPath = QDir(baseDir).filePath(bgImg);
        config_.cvBackgroundImage = QPixmap(imgPath);
    }
    ini.endGroup();

    ini.beginGroup(QStringLiteral("StatusBar"));
    config_.sbBackground = parseColor(ini.value(QStringLiteral("background_color"), "#FFFFFF").toString());
    config_.sbBorder = parseColor(ini.value(QStringLiteral("border_color"), "#D2D2D2").toString());
    config_.sbText = parseColor(ini.value(QStringLiteral("text_color"), "#323232").toString());
    config_.sbHover = parseColor(ini.value(QStringLiteral("hover_color"), "#00000012").toString());
    config_.sbLogo = parseColor(ini.value(QStringLiteral("logo_color"), "#FF7800").toString());
    config_.sbAi = parseColor(ini.value(QStringLiteral("ai_color"), "#2882DC").toString());
    config_.sbBorderRadius = ini.value(QStringLiteral("border_radius"), 6).toInt();

    QString sbBgImg = ini.value(QStringLiteral("background_image")).toString();
    if (!sbBgImg.isEmpty()) {
        QString imgPath = QDir(baseDir).filePath(sbBgImg);
        config_.sbBackgroundImage = QPixmap(imgPath);
    }
    ini.endGroup();

    return true;
}

QColor SkinLoader::parseColor(const QString &str) {
    if (str.isEmpty()) return QColor();

    if (str.startsWith(QStringLiteral("0x")) || str.startsWith(QStringLiteral("0X"))) {
        bool ok;
        uint val = str.mid(2).toUInt(&ok, 16);
        if (ok) {
            if (str.size() > 8) {
                return QColor::fromRgba(val);
            }
            return QColor::fromRgb(val);
        }
    }

    if (str.startsWith('#')) {
        QString hex = str.mid(1);
        if (hex.size() == 8) {
            bool ok;
            uint val = hex.toUInt(&ok, 16);
            if (ok) {
                int a = (val >> 24) & 0xFF;
                int r = (val >> 16) & 0xFF;
                int g = (val >> 8) & 0xFF;
                int b = val & 0xFF;
                return QColor(r, g, b, a);
            }
        }
        return QColor(str);
    }

    return QColor(str);
}

SkinConfig SkinLoader::darkSkin() {
    SkinConfig c;
    c.name = QStringLiteral("暗色皮肤");
    c.cvBackground = QColor(30, 30, 30, 220);
    c.cvBorder = QColor(80, 80, 80);
    c.cvText = QColor(240, 240, 240);
    c.cvHighlight = QColor(0, 180, 255);
    c.cvPreedit = QColor(255, 215, 0);
    c.cvIndex = QColor(150, 150, 150);
    c.cvSeparator = QColor(60, 60, 60);
    c.sbBackground = QColor(40, 40, 40, 240);
    c.sbBorder = QColor(70, 70, 70);
    c.sbText = QColor(230, 230, 230);
    c.sbHover = QColor(255, 255, 255, 30);
    c.sbLogo = QColor(255, 140, 26);
    c.sbAi = QColor(100, 200, 255);
    return c;
}

SkinConfig SkinLoader::lightSkin() {
    SkinConfig c;
    c.name = QStringLiteral("亮色皮肤");
    c.cvBackground = QColor(255, 255, 255, 245);
    c.cvBorder = QColor(210, 210, 210);
    c.cvText = QColor(51, 51, 51);
    c.cvHighlight = QColor(74, 144, 217);
    c.cvPreedit = QColor(255, 102, 0);
    c.cvIndex = QColor(153, 153, 153);
    c.cvSeparator = QColor(232, 232, 232);
    c.sbBackground = QColor(255, 255, 255, 245);
    c.sbBorder = QColor(210, 210, 210);
    c.sbText = QColor(50, 50, 50);
    c.sbHover = QColor(0, 0, 0, 18);
    c.sbLogo = QColor(255, 120, 0);
    c.sbAi = QColor(40, 130, 220);
    return c;
}

SkinConfig SkinLoader::blueSkin() {
    SkinConfig c;
    c.name = QStringLiteral("蓝色皮肤");
    c.cvBackground = QColor(25, 55, 110, 240);
    c.cvBorder = QColor(50, 100, 200);
    c.cvText = QColor(220, 235, 255);
    c.cvHighlight = QColor(100, 220, 255);
    c.cvPreedit = QColor(255, 200, 50);
    c.cvIndex = QColor(160, 190, 230);
    c.cvSeparator = QColor(50, 80, 150);
    c.sbBackground = QColor(25, 55, 110, 240);
    c.sbBorder = QColor(50, 100, 200);
    c.sbText = QColor(220, 235, 255);
    c.sbHover = QColor(255, 255, 255, 25);
    c.sbLogo = QColor(255, 180, 50);
    c.sbAi = QColor(150, 230, 255);
    return c;
}

SkinConfig SkinLoader::greenSkin() {
    SkinConfig c;
    c.name = QStringLiteral("草绿皮肤");
    c.cvBackground = QColor(34, 85, 51, 240);
    c.cvBorder = QColor(46, 125, 70);
    c.cvText = QColor(230, 255, 235);
    c.cvHighlight = QColor(120, 230, 150);
    c.cvPreedit = QColor(255, 220, 80);
    c.cvIndex = QColor(150, 210, 170);
    c.cvSeparator = QColor(50, 100, 65);
    c.sbBackground = QColor(34, 85, 51, 240);
    c.sbBorder = QColor(46, 125, 70);
    c.sbText = QColor(230, 255, 235);
    c.sbHover = QColor(255, 255, 255, 25);
    c.sbLogo = QColor(255, 220, 80);
    c.sbAi = QColor(150, 240, 180);
    return c;
}

SkinConfig SkinLoader::purpleSkin() {
    SkinConfig c;
    c.name = QStringLiteral("星紫皮肤");
    c.cvBackground = QColor(48, 30, 80, 240);
    c.cvBorder = QColor(90, 60, 140);
    c.cvText = QColor(235, 220, 255);
    c.cvHighlight = QColor(180, 130, 255);
    c.cvPreedit = QColor(255, 200, 120);
    c.cvIndex = QColor(180, 160, 220);
    c.cvSeparator = QColor(70, 50, 110);
    c.sbBackground = QColor(48, 30, 80, 240);
    c.sbBorder = QColor(90, 60, 140);
    c.sbText = QColor(235, 220, 255);
    c.sbHover = QColor(255, 255, 255, 25);
    c.sbLogo = QColor(255, 170, 100);
    c.sbAi = QColor(200, 160, 255);
    return c;
}

SkinConfig SkinLoader::roseSkin() {
    SkinConfig c;
    c.name = QStringLiteral("玫瑰皮肤");
    c.cvBackground = QColor(100, 20, 50, 240);
    c.cvBorder = QColor(160, 40, 80);
    c.cvText = QColor(255, 225, 235);
    c.cvHighlight = QColor(255, 120, 170);
    c.cvPreedit = QColor(255, 220, 100);
    c.cvIndex = QColor(220, 160, 180);
    c.cvSeparator = QColor(130, 40, 70);
    c.sbBackground = QColor(100, 20, 50, 240);
    c.sbBorder = QColor(160, 40, 80);
    c.sbText = QColor(255, 225, 235);
    c.sbHover = QColor(255, 255, 255, 25);
    c.sbLogo = QColor(255, 200, 50);
    c.sbAi = QColor(255, 150, 190);
    return c;
}

SkinConfig SkinLoader::sunsetSkin() {
    SkinConfig c;
    c.name = QStringLiteral("落霞皮肤");
    c.cvBackground = QColor(80, 35, 10, 240);
    c.cvBorder = QColor(160, 70, 20);
    c.cvText = QColor(255, 240, 220);
    c.cvHighlight = QColor(255, 160, 50);
    c.cvPreedit = QColor(255, 100, 60);
    c.cvIndex = QColor(220, 180, 140);
    c.cvSeparator = QColor(120, 55, 20);
    c.sbBackground = QColor(80, 35, 10, 240);
    c.sbBorder = QColor(160, 70, 20);
    c.sbText = QColor(255, 240, 220);
    c.sbHover = QColor(255, 255, 255, 25);
    c.sbLogo = QColor(255, 100, 60);
    c.sbAi = QColor(255, 190, 80);
    return c;
}

SkinConfig SkinLoader::iceSkin() {
    SkinConfig c;
    c.name = QStringLiteral("冰川皮肤");
    c.cvBackground = QColor(220, 240, 250, 245);
    c.cvBorder = QColor(180, 210, 230);
    c.cvText = QColor(30, 60, 80);
    c.cvHighlight = QColor(40, 140, 200);
    c.cvPreedit = QColor(0, 100, 180);
    c.cvIndex = QColor(100, 140, 170);
    c.cvSeparator = QColor(190, 215, 230);
    c.sbBackground = QColor(220, 240, 250, 245);
    c.sbBorder = QColor(180, 210, 230);
    c.sbText = QColor(30, 60, 80);
    c.sbHover = QColor(0, 0, 0, 15);
    c.sbLogo = QColor(0, 180, 220);
    c.sbAi = QColor(40, 140, 200);
    return c;
}
