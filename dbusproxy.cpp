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
#include "dbusproxy.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QWindow>
#include <QBoxLayout>

DBusProxy::DBusProxy(QObject *parent) : QObject(parent) {
}

bool DBusProxy::prepare() {
    QDBusConnection bus = QDBusConnection::sessionBus();
    QString service = QStringLiteral("org.dockbarx.LXQtApplet");
    QString path = QStringLiteral("/org/dockbarx/LXQtApplet");
    QString interface = service;
    iface = new QDBusInterface(service, path, interface, bus);
    if (bus.connect(service, path, interface, QStringLiteral("Ready"), this, SLOT(onReady(uint))) == false) {
        qWarning() << "Failed to connect to Ready signal";
        return false;
    }
    if (bus.connect(service, path, interface, QStringLiteral("SizeChanged"), this, SLOT(onSizeChanged(QList<int>))) == false) {
        qWarning() << "Failed to connect to SizeChanged signal";
        return false;
    }
    if (bus.connect(service, path, interface, QStringLiteral("Popup"), this, SLOT(onPopup(bool))) == false) {
        qWarning() << "Failed to connect to Popup signal";
        return false;
    }
    return true;
}

void DBusProxy::setPid(qint64 pid) {
    this->pid = pid;
}

bool DBusProxy::isRunning() {
    uint pid;
    return callGetPid(pid);
}

bool DBusProxy::validateSignal() {
    if (this->pid == 0) {
        return false;
    }
    uint pid;
    if (callGetPid(pid) == false) {
        return false;
    }
    return (pid == this->pid);
}

void DBusProxy::onReady(uint winId) {
    if (!validateSignal()) {
        return;
    }
    emit ready(winId);
}

void DBusProxy::onSizeChanged(const QList<int> &size) {
    if (!validateSignal()) {
        return;
    }
    if (size.size() != 2) {
        qWarning() << "onSizeChanged error: unexpected values";
        return;
    }
    emit sizeChanged(size[0], size[1]);
}

void DBusProxy::onPopup(bool shown) {
    if (!validateSignal()) {
        return;
    }
    emit popup(shown);
}

template<typename... Args>
bool callDBusMethod(QDBusInterface *iface, const QString &name, Args... args) {
    QDBusReply<void> reply = iface->call(name, args...);
    if (reply.isValid()) {
        return true;
    } else {
        QDBusError err = reply.error();
        qWarning() << "Call DBus method" << name << "failed:" << qPrintable(err.message());
        return false;
    }
}

bool DBusProxy::callSetSize(int size) {
    return callDBusMethod(iface, QStringLiteral("SetSize"), QVariant(size));
}

bool DBusProxy::callSetOrient(const QString &orient) {
    return callDBusMethod(iface, QStringLiteral("SetOrient"), QVariant(orient));
}

bool DBusProxy::callSetBgImage(const QString &image, int offsetX, int offsetY) {
    return callDBusMethod(iface, QStringLiteral("SetBgImage"), QVariant(image), QVariant(offsetX), QVariant(offsetY));
}

bool DBusProxy::callSetBgColor(const QString &color) {
    return callDBusMethod(iface, QStringLiteral("SetBgColor"), QVariant(color));
}

bool DBusProxy::callGetPid(uint &pid) {
    QDBusReply<uint> reply = iface->call(QStringLiteral("GetPid"));
    if (reply.isValid()) {
        pid = reply.value();
        return true;
    }
    return false;
}