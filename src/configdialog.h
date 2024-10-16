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
#include <QProcess>

class QSpinBox;
class QCheckBox;
class QDialogButtonBox;
class QAbstractButton;
class LXQtPluginSettings;

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(LXQtPluginSettings *settings, QWidget *parent = nullptr);

private slots:
    void updateOffset(int value);
    void updateMaxSize(int value);
    void onCheck();
    void onButton(QAbstractButton *button);

private:
    void initSettings();

    static const QString dbx_pref;

    LXQtPluginSettings *settings;

    QSpinBox *offsetBox;
    QCheckBox *maxSizeCheck;
    QSpinBox *maxSizeBox;
    QDialogButtonBox *buttons;
};

#endif // CONFIGDIALOG_H
