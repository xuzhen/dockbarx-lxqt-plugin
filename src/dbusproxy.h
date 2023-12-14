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
#ifndef DBUSPROXY_H
#define DBUSPROXY_H

#include <QObject>

class QDBusInterface;

class DBusProxy : public QObject
{
    Q_OBJECT
public:
    explicit DBusProxy(QObject *parent = nullptr);
    ~DBusProxy();

    void setPid(qint64 pid);

    bool isRunning();

    bool callSetSize(int size);
    bool callSetOrient(const QString &orient);
    bool callSetBgImage(const QString &image, int offsetX, int offsetY);
    bool callSetBgColor(const QString &color);

signals:
    void ready(uint winId);
    void sizeChanged(int width, int height);
    void popup(bool shown);

private slots:
    void onReady(uint winId);
    void onSizeChanged(const QList<int> &size);
    void onPopup(bool shown);
    void onNameOwnerChanged(const QString &name, const QString &previousOwner, const QString &currentOwner);

private:
    uint getDbxPid();
    QDBusInterface *dbxIface;
    QDBusInterface *ofdIface;
    quint64 pid = 0;
    uint nameOwnerPid = 0;
    const QString dbxDBusName = QStringLiteral("org.dockbarx.LXQtApplet");
};

#endif // DBUSPROXY_H
