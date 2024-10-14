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
#include "pyappletkeeper.h"
#include <QScreen>
#include "config.h"
#include "dbusproxy.h"

PyAppletKeeper::PyAppletKeeper(QScreen *screen, QObject *parent) : QObject(parent), screen(screen) {
    proc.setProgram(QStringLiteral(u"%1/lxqt-panel-applet.py").arg(DOCKBARX_PATH));
    proc.setWorkingDirectory(DOCKBARX_PATH);

    // LXQt Panel ignore the GTK scaling setting, so do we
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GDK_SCALE", "1");
    proc.setProcessEnvironment(env);

    connect(&proc, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, &PyAppletKeeper::start);

    connect(screen, &QScreen::physicalDotsPerInchChanged, this, &PyAppletKeeper::resize);

    timer.setInterval(1000);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, &PyAppletKeeper::start);

    connect(&dbus, &DBusProxy::ready, this, &PyAppletKeeper::dockReady);
    connect(&dbus, &DBusProxy::sizeChanged, this, &PyAppletKeeper::dockSizeChanged);
    connect(&dbus, &DBusProxy::popup, this, &PyAppletKeeper::dockPopup);
}

PyAppletKeeper::~PyAppletKeeper() {
    stop();
}

bool PyAppletKeeper::setDockOrient(const QString &orient) {
    if (this->orient != orient) {
        if (proc.state() == QProcess::Running) {
            if (dbus.callSetOrient(orient) == false) {
                return false;
            }
        }
        this->orient = orient;
    }
    return true;
}

bool PyAppletKeeper::setDockSize(int size) {
    if (this->size != size) {
        if (proc.state() == QProcess::Running) {
            if (dbus.callSetSize(size)) {
                return false;
            }
        }
        this->size = size;
    }
    return true;
}

bool PyAppletKeeper::setDockIconTheme(const QString &iconTheme) {
    if (this->iconTheme != iconTheme) {
        if (proc.state() == QProcess::Running) {
            if (dbus.callSetIconTheme(iconTheme)) {
                return false;
            }
        }
        this->iconTheme = iconTheme;
    }
    return true;
}

bool PyAppletKeeper::setDockBackground(const QString &color, const QString &image, int offsetX, int offsetY, int panelWidth, int panelHeight) {
    return dbus.callSetBackground(color, image, offsetX, offsetY, panelWidth, panelHeight);
}

QStringList PyAppletKeeper::getArguments() {
    QStringList list;
    if (orient.isEmpty() == false) {
        list << QStringLiteral(u"-o") << orient;
    }
    if (size > 0) {
        list << QStringLiteral(u"-s") << QString::number(size);
    }
    if (iconTheme.isEmpty() == false) {
        list << QStringLiteral(u"-i") << iconTheme;
    }
    double factor = screen->devicePixelRatio();
    if (factor != 1.0) {
        list << QStringLiteral(u"-f") << QString::number(factor);
    }
    return list;
}

void PyAppletKeeper::start() {
    if (dbus.isRunning() == false) {
        proc.setArguments(getArguments());
        proc.start();
        dbus.setPid(proc.processId());
    } else {
        timer.start();
        dbus.setPid(0);
    }
}

void PyAppletKeeper::stop() {
    timer.stop();
    proc.disconnect();
    proc.kill();
    proc.waitForFinished();
}

void PyAppletKeeper::resize() {
    if (proc.state() == QProcess::Running) {
        dbus.callSetSize(size);
    }
}

