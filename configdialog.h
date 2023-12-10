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
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent = nullptr);
    virtual ~ConfigDialog();

    void setVisible(bool visible) override;

private:
    QString dbx_pref = QStringLiteral(u"dbx_preference");
    qint64 pid = 0;
};

#endif // CONFIGDIALOG_H
