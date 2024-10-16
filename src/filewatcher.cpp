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
#include "filewatcher.h"
#include <QThread>
#include <QFileInfo>
#include <QDir>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtLogging>
#else
#include <QDebug>
#endif
#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>

FileWatcher::FileWatcher(const QString &file, QObject *parent) : QObject(parent), filePath(file) {
    fd = inotify_init();
    if (fd == -1) {
        qWarning() << "Failed to init inotify:" << qPrintable(strerror(errno));
    }
    mtime = QFileInfo(file).lastModified();
}

FileWatcher::~FileWatcher() {
    stop();
    delete timer;
    if (fd != -1) {
        close(fd);
    }
}

void FileWatcher::run() {
    QFileInfo info(filePath);
    QString dir = info.dir().path();
    QString fileName = info.fileName();

    int wd = inotify_add_watch(fd, dir.toLocal8Bit(), IN_MODIFY | IN_MOVED_TO);
    if (wd == -1) {
        qWarning() << "Failed to add inotify watch:" << qPrintable(strerror(errno));
        startTimer();
        return;
    }
    char buf[sizeof(struct inotify_event) + NAME_MAX + 1];

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    isRunning = true;
    while (isRunning) {
        int nfd = poll(fds, 1, 100);
        if (nfd == -1) {
            qWarning() << "Failed to poll inotify event:" << qPrintable(strerror(errno));
            startTimer();
            isRunning = false;
            break;
        } else if (nfd == 0) {
            continue;
        }
        if (read(fd, buf, sizeof(buf)) == -1) {
            qWarning() << "Failed to read inotify event:" << qPrintable(strerror(errno));
            continue;
        }
        inotify_event *event = reinterpret_cast<inotify_event*>(buf);
        if (QString::fromLocal8Bit(event->name) != fileName) {
            continue;
        }
        emit modified();
    }
    inotify_rm_watch(fd, wd);
    thread()->quit();
}

void FileWatcher::start() {
    if (fd == -1) {
        startTimer();
    } else {
        QThread *thread = new QThread();
        moveToThread(thread);
        connect(thread, &QThread::started, this, &FileWatcher::run);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        thread->start();
    }
}

void FileWatcher::stop() {
    if (isRunning) {
        isRunning = false;
    } else if (timer != nullptr) {
        timer->stop();
    }
}

void FileWatcher::startTimer() {
    if (timer == nullptr) {
        timer = new QTimer();
        timer->setInterval(100);
        timer->setSingleShot(false);
        connect(timer, &QTimer::timeout, this, &FileWatcher::checkFile);
    }
    timer->start();
}

void FileWatcher::checkFile() {
    QFileInfo info(filePath);
    QDateTime time = info.lastModified();
    if (mtime != time) {
        mtime = time;
        emit modified();
    }
}