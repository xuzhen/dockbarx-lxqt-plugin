#include "panelsettingswatcher.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>

PanelSettingsWatcher::PanelSettingsWatcher(const QString &file, QObject *parent) : QObject(parent), filePath(file) {
    fd = inotify_init();
    if (fd == -1) {
        qWarning() << "Failed to init inotify:" << qPrintable(strerror(errno));
    }
    timer.setInterval(100);
    timer.setSingleShot(false);
    connect(&timer, &QTimer::timeout, this, &PanelSettingsWatcher::checkFile);
}

PanelSettingsWatcher::~PanelSettingsWatcher() {
    stop();
}

void PanelSettingsWatcher::start() {
    if (fd == -1) {
        timer.start();
    } else {
        QFileInfo info(filePath);
        QString dir = info.dir().path();
        QString fileName = info.fileName();

        int wd = inotify_add_watch(fd, dir.toLocal8Bit(), IN_MODIFY | IN_MOVED_TO);
        if (wd == -1) {
            qWarning() << "Failed to add inotify watch:" << qPrintable(strerror(errno));
            timer.start();
            return;
        }
        char buf[sizeof(struct inotify_event) + NAME_MAX + 1];

        struct pollfd fds[1];
        fds[0].fd = fd;
        fds[0].events = POLLIN;

        run = true;
        while (run) {
            int nfd = poll(fds, 1, 100);
            if (nfd == -1) {
                qWarning() << "Failed to poll inotify event:" << qPrintable(strerror(errno));
                timer.start();
                run = false;
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
    }
}

void PanelSettingsWatcher::stop() {
    if (run) {
        run = false;
    } else {
        timer.stop();
    }
}

void PanelSettingsWatcher::checkFile() {
    QFileInfo info(filePath);
    QDateTime time = info.lastModified();
    if (mtime != time) {
        mtime = time;
        emit modified();
    }
}