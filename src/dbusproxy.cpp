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
#include "dbusproxy.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QWindow>
#include <QBoxLayout>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtLogging>
#else
#include <QDebug>
#endif

DBusProxy::DBusProxy(QObject *parent) : QObject(parent) {
    QDBusConnection bus = QDBusConnection::sessionBus();

    QString service = QStringLiteral("org.freedesktop.DBus");
    QString path = QStringLiteral("/org/freedesktop/DBus");
    QString interface = service;
    ofdIface = new QDBusInterface(service, path, interface, bus);
    if (bus.connect(service, path, interface, QStringLiteral("NameOwnerChanged"), this, SLOT(onNameOwnerChanged(QString,QString,QString))) == false) {
        qWarning() << "Failed to connect to NameOwnerChanged signal";
    }

    service = dbxDBusName;
    path = QStringLiteral("/org/dockbarx/LXQtApplet");
    interface = service;
    dbxIface = new QDBusInterface(service, path, interface, bus);
    if (bus.connect(service, path, interface, QStringLiteral("Ready"), this, SLOT(onReady(uint))) == false) {
        qWarning() << "Failed to connect to Ready signal";
    }
    if (bus.connect(service, path, interface, QStringLiteral("SizeChanged"), this, SLOT(onSizeChanged(QList<int>))) == false) {
        qWarning() << "Failed to connect to SizeChanged signal";
    }
    if (bus.connect(service, path, interface, QStringLiteral("Popup"), this, SLOT(onPopup(bool))) == false) {
        qWarning() << "Failed to connect to Popup signal";
    }
}

DBusProxy::~DBusProxy() {
    delete dbxIface;
    delete ofdIface;
}

void DBusProxy::setPid(qint64 pid) {
    this->pid = pid;
}

bool DBusProxy::isRunning() {
    return getDbxPid() != 0;
}

void DBusProxy::onReady(uint winId) {
    if (pid == 0 || nameOwnerPid != pid) {
        return;
    }
    emit ready(winId);
}

void DBusProxy::onSizeChanged(const QList<int> &size) {
    if (pid == 0 || nameOwnerPid != pid) {
        return;
    }
    if (size.size() != 2) {
        qWarning() << "onSizeChanged error: unexpected values";
        return;
    }
    emit sizeChanged(size[0], size[1]);
}

void DBusProxy::onPopup(bool shown) {
    if (pid == 0 || nameOwnerPid != pid) {
        return;
    }
    emit popup(shown);
}

void DBusProxy::onNameOwnerChanged(const QString &name, const QString &previousOwner, const QString &currentOwner) {
    if (name == QStringLiteral("org.dockbarx.LXQtApplet")) {
        if (currentOwner.isEmpty()) {
            nameOwnerPid = 0;
        } else {
            nameOwnerPid = getDbxPid();
        }
    }
}

uint DBusProxy::getDbxPid() {
    QDBusReply<uint> reply = ofdIface->call(QStringLiteral("GetConnectionUnixProcessID"), QVariant(dbxDBusName));
    if (reply.isValid()) {
        return reply.value();
    }
    return 0;
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
    if (pid == 0 || nameOwnerPid != pid) {
        return false;
    }
    return callDBusMethod(dbxIface, QStringLiteral("SetSize"), QVariant(size));
}

bool DBusProxy::callSetOrient(const QString &orient) {
    if (pid == 0 || nameOwnerPid != pid) {
        return false;
    }
    return callDBusMethod(dbxIface, QStringLiteral("SetOrient"), QVariant(orient));
}

bool DBusProxy::callSetBackground(const QString &color, const QString &image, int offsetX, int offsetY, int panelWidth, int panelHeight) {
    if (pid == 0 || nameOwnerPid != pid) {
        return false;
    }
    return callDBusMethod(dbxIface, QStringLiteral("SetBackground"), QVariant(color), QVariant(image), QVariant(offsetX), QVariant(offsetY), QVariant(panelWidth), QVariant(panelHeight));
}

bool DBusProxy::callSetIconTheme(const QString &themeName) {
    if (pid == 0 || nameOwnerPid != pid) {
        return false;
    }
    return callDBusMethod(dbxIface, QStringLiteral("SetIconTheme"), QVariant(themeName));
}

bool DBusProxy::callSetScalingFactor(double factor) {
    if (pid == 0 || nameOwnerPid != pid) {
        return false;
    }
    return callDBusMethod(dbxIface, QStringLiteral("SetScalingFactor"), QVariant(factor));
}
