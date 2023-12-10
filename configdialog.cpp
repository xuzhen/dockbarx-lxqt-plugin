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
#include "configdialog.h"
#include <QProcess>
#include <QDebug>
#include <signal.h>

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent) {
    setMaximumSize(0, 0);
    move(-100, -100);
}

ConfigDialog::~ConfigDialog() {
    if (pid != 0) {
        kill(pid, SIGTERM);
    }
}

void ConfigDialog::setVisible(bool visible) {
    QDialog::setVisible(visible);
    if (visible) {
        if (QProcess::startDetached(dbx_pref, QStringList(), QStringLiteral(u"."), &pid) == false) {
            qWarning() << "Failed to run" << dbx_pref;
        }
        QDialog::setVisible(false);
    } else {
        kill(pid, SIGTERM);
        pid = 0;
    }
}
