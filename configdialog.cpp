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
#include <QMessageBox>

const QString ConfigDialog::dbx_pref = QStringLiteral(u"dbx_preference");

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent) {
    setMaximumSize(0, 0);
    setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
    move(-100, -100);
    proc.setProgram(dbx_pref);
    connect(&proc, &QProcess::errorOccurred, this, &ConfigDialog::onError);
}

ConfigDialog::~ConfigDialog() {
    if (proc.processId() != 0) {
        proc.kill();
    }
}

void ConfigDialog::setVisible(bool visible) {
    QDialog::setVisible(visible);
    if (visible) {
        proc.start();
        QDialog::setVisible(false);
    } else {
        if (proc.processId() != 0) {
            proc.kill();
        }
    }
}

void ConfigDialog::onError(QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart) {
        QMessageBox::critical(this->parentWidget(), QStringLiteral(u"Error"), QStringLiteral(u"Failed to run ") + dbx_pref);
    }
}

