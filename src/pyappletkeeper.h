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
#ifndef PYAPPLETKEEPER_H
#define PYAPPLETKEEPER_H

#include <QProcess>
#include <QTimer>
#include <QMap>
#include "dbusproxy.h"

class QScreen;

class PyAppletKeeper : public QObject
{
    Q_OBJECT
public:
    explicit PyAppletKeeper(QScreen *screen, QObject *parent = nullptr);
    ~PyAppletKeeper();

    bool setDockOrient(const QString &orient);
    bool setDockSize(int size);
    bool setDockIconTheme(const QString &iconTheme);
    bool setDockMaxSize(int size);

    bool setDockBackground(const QString &color, const QString &image, int offsetX, int offsetY, int panelWidth, int panelHeight);

    void stop();

public slots:
    void start();

signals:
    void dockReady(uint winId);
    void dockSizeChanged(int width, int height);
    void dockPopup(bool shown);

private slots:
    void setScalingFactor();

private:
    QStringList getArguments();

    QString orient;
    int size = 0;
    int maxSize = -1;
    QString iconTheme;

    QScreen *screen;
    QProcess proc;
    QTimer timer;
    DBusProxy dbus;
};

#endif // PYAPPLETKEEPER_H
