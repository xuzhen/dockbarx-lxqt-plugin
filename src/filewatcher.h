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
#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>

class QThread;

class FileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcher(const QString &file, QObject *parent = nullptr);
    ~FileWatcher();
    void start();
    void stop();

signals:
    void modified();

private slots:
    void run();
    void checkFile();

private:
    void startTimer();
    volatile bool isRunning = false;
    int fd;
    QString filePath;
    QTimer *timer = nullptr;
    QDateTime mtime;
};

#endif // FILEWATCHER_H
