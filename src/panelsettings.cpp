/*
 Copyright (C) 2023-2024 Xu Zhen

 This file is part of DockbarX LXQt panel plugin.

 DockbarX LXQt panel plugin is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.

 DockbarX LXQt panel plugin is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this file. If not, see <http://www.gnu.org/licenses/>.
*/
#include "panelsettings.h"
#include <QStandardPaths>
#include <QSettings>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QCommandLineParser>
#include <QColor>
#include <QThread>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtLogging>
#else
#include <QDebug>
#endif
#include <lxqt/LXQt/lxqtapplication.h>
#include "filewatcher.h"
#include "themeparser.h"

PanelSettings::PanelSettings(const QString &panelName, QObject *parent) : QObject(parent) {
    QString file = findSettingFile();
    settings = new QSettings(file, QSettings::IniFormat);
    group = findPanelGroup(panelName);
    initValues();

    watcher = new FileWatcher(file);
    watcher->start();
    connect(watcher, &FileWatcher::modified, this, &PanelSettings::modified, Qt::QueuedConnection);

    themeColor = ThemeParser(lxqtApp->styleSheet()).getBackgroundColor();
    connect(lxqtApp, &LXQt::Application::themeChanged, this, &PanelSettings::onThemeChanged);
}

PanelSettings::~PanelSettings() {
    watcher->stop();
    watcher->deleteLater();
    delete settings;
}

QColor PanelSettings::getBackgroundColor() {
    QColor c;
    if (color.isValid()) {
        c = color;
        c.setAlphaF(opacity / 100.0);
    } else if (themeColor.isValid()) {
        c = themeColor;
    } else {
        c.setAlpha(0);
    }
    return c;
}

QString PanelSettings::getBackgroundImage() {
    return image;
}

QString PanelSettings::getIconTheme() {
    return iconTheme;
}

void PanelSettings::modified() {
    settings->sync();
    QString iconTheme = readIconTheme();
    if (this->iconTheme != iconTheme) {
        this->iconTheme = iconTheme;
        emit iconThemeChanged(iconTheme);
    }
    settings->beginGroup(group);
        QString image = readBackgroundImage();
        QColor color = readBackgroundColor();
        int opacity = readOpacity();
    settings->endGroup();
    if (this->image != image || this->color != color || this->opacity != opacity) {
        this->image = image;
        this->color = color;
        this->opacity = opacity;
        emit backgroundChanged(image, getBackgroundColor());
    }
}

QString PanelSettings::findSettingFile() {
    QCommandLineParser parser;
    QCommandLineOption configFileOption(QStringList() << QLatin1String("c") << QLatin1String("config") << QLatin1String("configfile"), QLatin1String("c"), QLatin1String("c"));
    parser.addOption(configFileOption);
    if (parser.parse(qApp->arguments())) {
        QString file = parser.value(configFileOption);
        if (file.isEmpty() == false) {
            return file;
        }
    }
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral(u"/lxqt/panel.conf");
}

QString PanelSettings::findPanelGroup(const QString &panelName) {
    static const QRegularExpression nameRe(QStringLiteral(u"^LXQtPanel (panel(\\d+|_\\{[0-9A-Fa-f]{8}(-[0-9A-Fa-f]{4}){3}-[0-9A-Fa-f]{12}\\}))(Window)?$"));
    static const QRegularExpression groupRe(QStringLiteral(u"^panel(\\d+|_\\{[0-9A-Fa-f]{8}(-[0-9A-Fa-f]{4}){3}-[0-9A-Fa-f]{12}\\})$"));

    QString panelGroup;
    QRegularExpressionMatch m = nameRe.match(panelName);
    if (m.hasMatch()) {
        panelGroup = m.captured(1);
    } else {
        qWarning() << "LXQtPanel object name pattern changed:" << panelName;
        const QStringList groups = settings->childGroups();
        for (const QString &group : groups) {
            if (groupRe.match(group).hasMatch()) {
                settings->beginGroup(group);
                QStringList plugins = settings->value(QStringLiteral(u"plugins")).toString().remove(QChar(' ')).split(QChar(','));
                settings->endGroup();
                if (plugins.contains(QStringLiteral(u"dockbarx"))) {
                    panelGroup = group;
                    break;
                }
            }
        }
        if (panelGroup.isEmpty()) {
            panelGroup = QStringLiteral(u"panel1");
        }
    }
    return panelGroup;
}

void PanelSettings::initValues() {
    settings->sync();
    iconTheme = readIconTheme();
    settings->beginGroup(group);
        opacity = readOpacity();
        color = readBackgroundColor();
        image = readBackgroundImage();
    settings->endGroup();
}

int PanelSettings::readOpacity() {
    return settings->value(QStringLiteral(u"opacity"), QVariant(100)).toInt();
}

QColor PanelSettings::readBackgroundColor() {
    return settings->value(QStringLiteral(u"background-color"), QVariant(QColor())).value<QColor>();
}

QString PanelSettings::readBackgroundImage() {
    return settings->value(QStringLiteral(u"background-image")).toString();
}

QString PanelSettings::readIconTheme() {
    return settings->value(QStringLiteral(u"iconTheme")).toString();
}

void PanelSettings::onThemeChanged() {
    ThemeParser parser(lxqtApp->styleSheet());
    QColor c = parser.getBackgroundColor();
    if (themeColor != c) {
        themeColor = c;
        if (!color.isValid() && image.isEmpty()) {
            emit backgroundChanged(image, getBackgroundColor());
        }
    }
}


