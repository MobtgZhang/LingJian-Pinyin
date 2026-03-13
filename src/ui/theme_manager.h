#pragma once

#include "skin_loader.h"

#include <QColor>
#include <QFont>
#include <QObject>
#include <QString>

class ThemeManager : public QObject {
    Q_OBJECT
public:
    static ThemeManager &instance();

    void applySkin(const SkinConfig &skin);
    bool loadSkinFromZip(const QString &zipPath);
    bool loadSkinFromDirectory(const QString &dirPath);

    void setBuiltinSkin(const QString &name);

    const SkinConfig &skin() const { return skin_; }

signals:
    void skinChanged();

private:
    ThemeManager();
    SkinConfig skin_;
    SkinLoader loader_;
};
