/*
 Copyright (C) 2023 Xu Zhen

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
#ifndef PANELSETTINGSWATCHER_H
#define PANELSETTINGSWATCHER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>

class PanelSettingsWatcher : public QObject
{
    Q_OBJECT
public:
    explicit PanelSettingsWatcher(const QString &file, QObject *parent = nullptr);
    ~PanelSettingsWatcher();
    void stop();

public slots:
    void start();

signals:
    void modified();

private slots:
    void checkFile();

private:
    void startTimer();
    volatile bool run = false;
    int fd;
    QString filePath;
    QTimer *timer = nullptr;
    QDateTime mtime;
};

#endif // PANELSETTINGSWATCHER_H
