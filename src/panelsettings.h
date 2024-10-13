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
#ifndef PANELSETTINGS_H
#define PANELSETTINGS_H

#include <QObject>
#include <QColor>

class QSettings;
class FileWatcher;

class PanelSettings : public QObject
{
    Q_OBJECT
public:
    explicit PanelSettings(const QString &panelName, QObject *parent = nullptr);
    ~PanelSettings();

    QString getBackgroundColor();
    QString getBackgroundImage();
    QString getIconTheme();

    bool isFixedBackground();

signals:
    void positionChanged();
    void backgroundChanged(QString image, QString color);
    void iconThemeChanged(QString themeName);

private slots:
    void modified();
    void onThemeChanged();

private:
    QString findSettingFile();
    QString findPanelGroup(const QString &panelName);
    void initValues();
    int readOpacity();
    QColor readBackgroundColor();
    QString readBackgroundImage();
    QString readIconTheme();
    QString readPostion();

    QSettings *settings;
    QString group;

    QString iconTheme;
    int opacity;
    QColor color;
    QString image;

    QString position;

    QColor themeColor;
    QString themeLinearGradient;

    FileWatcher *watcher;
};

#endif // PANELSETTINGS_H
