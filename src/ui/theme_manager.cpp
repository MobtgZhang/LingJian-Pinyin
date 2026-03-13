#include "theme_manager.h"

ThemeManager &ThemeManager::instance() {
    static ThemeManager mgr;
    return mgr;
}

ThemeManager::ThemeManager() {
    skin_ = SkinLoader::lightSkin();
}

void ThemeManager::applySkin(const SkinConfig &skin) {
    skin_ = skin;
    emit skinChanged();
}

bool ThemeManager::loadSkinFromZip(const QString &zipPath) {
    if (loader_.loadFromZip(zipPath)) {
        skin_ = loader_.config();
        emit skinChanged();
        return true;
    }
    return false;
}

bool ThemeManager::loadSkinFromDirectory(const QString &dirPath) {
    if (loader_.loadFromDirectory(dirPath)) {
        skin_ = loader_.config();
        emit skinChanged();
        return true;
    }
    return false;
}

void ThemeManager::setBuiltinSkin(const QString &name) {
    if (name == QStringLiteral("dark")) {
        skin_ = SkinLoader::darkSkin();
    } else if (name == QStringLiteral("blue")) {
        skin_ = SkinLoader::blueSkin();
    } else if (name == QStringLiteral("green")) {
        skin_ = SkinLoader::greenSkin();
    } else if (name == QStringLiteral("purple")) {
        skin_ = SkinLoader::purpleSkin();
    } else if (name == QStringLiteral("rose")) {
        skin_ = SkinLoader::roseSkin();
    } else if (name == QStringLiteral("sunset")) {
        skin_ = SkinLoader::sunsetSkin();
    } else if (name == QStringLiteral("ice")) {
        skin_ = SkinLoader::iceSkin();
    } else {
        skin_ = SkinLoader::lightSkin();
    }
    emit skinChanged();
}
