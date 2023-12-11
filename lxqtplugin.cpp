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
#include "lxqtplugin.h"
#include <lxqt/ilxqtpanel.h>
#include <lxqt/lxqtpanelglobals.h>
#include <lxqt/pluginsettings.h>
#include <QX11Info>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTime>
#include <QBoxLayout>
#include <QWindow>
#include <QWidget>
#include <QProcess>
#include <QRgb>
#include <QDebug>
#include <signal.h>
#include "config.h"
#include "configdialog.h"
#include "panelsettings.h"

LXQtPlugin::LXQtPlugin(const ILXQtPanelPluginStartupInfo &startupInfo) :
    QObject(),
    ILXQtPanelPlugin(startupInfo)
{
    wrapper = new QWidget();
    auto layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setContentsMargins(0, 0, 0, 0);
    wrapper->setLayout(layout);

    pconf = new PanelSettings();
    connect(pconf, &PanelSettings::backgroundChanged, this, &LXQtPlugin::onBackgroundChanged);
}

LXQtPlugin::~LXQtPlugin() {
    delete pconf;
    delete wrapper;
}

QWidget *LXQtPlugin::widget() {
    prepareDBus();
    runPythonApplet();
    return wrapper;
}

QDialog *LXQtPlugin::configureDialog() {
    return new ConfigDialog();
}

void LXQtPlugin::realign() {
    QString orient = getOrient();
    if (remoteOrient != orient) {
        if (dbusSetOrient(orient)) {
            remoteOrient = orient;
        }
    }
    int size = getPanelSize();
    if (remoteSize != size) {
        if (dbusSetSize(size)) {
            remoteSize = size;
        }
    }
    QPoint pos = wrapper->mapToGlobal(QPoint(0, 0));
    if (this->pos != pos) {
        this->pos = pos;
        if (pconf->getBackgroundImage().isEmpty() == false) {
            setBackground();
        }
    }
}

QString LXQtPlugin::getOrient() {
    ILXQtPanel::Position pos = panel()->position();
    switch (pos) {
    case ILXQtPanel::PositionBottom:
        return QStringLiteral(u"down");
    case ILXQtPanel::PositionTop:
        return QStringLiteral(u"up");
    case ILXQtPanel::PositionLeft:
        return QStringLiteral(u"left");
    case ILXQtPanel::PositionRight:
        return QStringLiteral(u"right");
    }
    return QString();
}

int LXQtPlugin::getPanelSize() {
    QRect geo = panel()->globalGeometry();
    if (panel()->isHorizontal()) {
        return geo.height();
    } else {
        return geo.width();
    }
}

void LXQtPlugin::setBackground() {
    QString image = pconf->getBackgroundImage();
    QColor color = pconf->getBackgroundColor();
    int opacity = pconf->getOpacity();
    onBackgroundChanged(image, color, opacity);
}

bool LXQtPlugin::runPythonApplet() {
    remoteOrient = getOrient();
    // Not fully initialized yet, getPanelSize() returns incorrect size
    remoteSize = pconf->getPanelSize();
    QString path = QStringLiteral(u"%1/lxqt-panel-plugin.py").arg(DOCKBARX_PATH);
    QStringList args;
    args << QStringLiteral(u"-o") << remoteOrient <<
            QStringLiteral(u"-s") << QString::number(remoteSize);
    qint64 pid;
    if (QProcess::startDetached(path, args, DOCKBARX_PATH, &pid) == false) {
        qWarning() << "Failed to run" << path;
        return false;
    }
    QObject::connect(wrapper, &QWidget::destroyed, [pid]{
        kill(pid, SIGINT);
    });
    return true;
}

bool LXQtPlugin::prepareDBus() {
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
    return true;
}

void LXQtPlugin::onReady(uint winId) {
    QWindow *win = QWindow::fromWinId(winId);
    wrapper->layout()->addWidget(QWidget::createWindowContainer(win));
    setBackground();
}

void LXQtPlugin::onSizeChanged(const QList<int> &size) {
    if (size.size() != 2) {
        qWarning() << "onSizeChanged error: unexpected values";
        return;
    }
    wrapper->setMinimumSize(size[0], size[1]);
}

void LXQtPlugin::onBackgroundChanged(const QString &image, const QColor &color, int opacity) {
    if (image.isEmpty() == false) {
        QPoint pluginPos = wrapper->mapToGlobal(QPoint(0, 0));
        QRect panelPos = panel()->globalGeometry();
        int offsetX = pluginPos.x() - panelPos.x();
        int offsetY = pluginPos.y() - panelPos.y();
        dbusSetBgImage(image, offsetX, offsetY);
    } else {
        if (color.isValid() == false) {
            opacity = 0;
        }
        QString rgba = QStringLiteral(u"rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(opacity / 100.0);
        dbusSetBgColor(rgba);
    }
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

bool LXQtPlugin::dbusSetSize(int size) {
    return callDBusMethod(iface, QStringLiteral("SetSize"), QVariant(size));
}

bool LXQtPlugin::dbusSetOrient(const QString &orient) {
    return callDBusMethod(iface, QStringLiteral("SetOrient"), QVariant(orient));
}

bool LXQtPlugin::dbusSetBgImage(const QString &image, int offsetX, int offsetY) {
    return callDBusMethod(iface, QStringLiteral("SetBgImage"), QVariant(image), QVariant(offsetX), QVariant(offsetY));
}

bool LXQtPlugin::dbusSetBgColor(const QString &color) {
    return callDBusMethod(iface, QStringLiteral("SetBgColor"), QVariant(color));
}