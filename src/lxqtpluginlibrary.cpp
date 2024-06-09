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
#include "lxqtpluginlibrary.h"
#include <QDBusConnection>
#if QT_VERSION_MAJOR == 5
#include <QX11Info>
#else
#include <QGuiApplication>
#endif
#include <QDebug>
#include "lxqtplugin.h"

ILXQtPanelPlugin *LXQtPluginLibrary::instance(const ILXQtPanelPluginStartupInfo &startupInfo) const {
#if QT_VERSION_MAJOR == 5
    if (!QX11Info::connection()) {
#else
    if (!qApp->nativeInterface<QNativeInterface::QX11Application>()->connection()) {
#endif
        qWarning() << "DockbarX plugin supports X11 only. Skipping.";
        return nullptr;
    }
    if (QDBusConnection::sessionBus().isConnected() == false) {
        qWarning() << "DBus not ready";
        return nullptr;
    }
    return new LXQtPlugin(startupInfo);
}
