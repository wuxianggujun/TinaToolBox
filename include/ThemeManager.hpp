/*
#pragma once

#include "Singleton.hpp"
#include <QString>
#include <QColor>
#include <QMap>
#include <QObject>

namespace TinaToolBox {
    class ThemeManager : public QObject, public Singleton<ThemeManager> {
        Q_OBJECT
        friend class Singleton<ThemeManager>;

    public:
        void loadTheme(const QString &themeName);

        QColor getColor(const QString &colorName) const;

        QString getStyleSheet(const QString &styleSheetName) const;

    signals:
        void themeChanged();

    private:
        ThemeManager() : QObject(nullptr) {
        }

        QString currentTheme_;
        QMap<QString, QColor> colors_;
        QMap<QString, QString> stylesheets_;
    };
}
*/
