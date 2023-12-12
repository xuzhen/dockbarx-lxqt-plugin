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
#include "pyappletkeeper.h"
#include "config.h"
#include "dbusproxy.h"

PyAppletKeeper::PyAppletKeeper(DBusProxy *dbus, QObject *parent) : QObject(parent), dbus(dbus) {
    proc.setProgram(QStringLiteral(u"%1/lxqt-panel-plugin.py").arg(DOCKBARX_PATH));
    proc.setWorkingDirectory(DOCKBARX_PATH);
    QObject::connect(&proc, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, &PyAppletKeeper::start);

    timer.setInterval(1000);
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, this, &PyAppletKeeper::start);
}

PyAppletKeeper::~PyAppletKeeper() {
    stop();
}

void PyAppletKeeper::setStartupArguments(const QString &orient, int size) {
    args.clear();
    args << QStringLiteral(u"-o") << orient;
    args << QStringLiteral(u"-s") << QString::number(size);
}

void PyAppletKeeper::start() {
    if (dbus->isRunning() == false) {
        proc.setArguments(args);
        proc.start();
        dbus->setPid(proc.processId());
    } else {
        timer.start();
        dbus->setPid(0);
    }
}

void PyAppletKeeper::stop() {
    timer.stop();
    proc.disconnect();
    proc.kill();
    proc.waitForFinished();
}
