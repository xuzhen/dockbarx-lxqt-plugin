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
#include <QTime>
#include <QBoxLayout>
#include <QWindow>
#include <QWidget>
#include <QProcess>
#include <QRgb>
#include <QDebug>
#include "configdialog.h"
#include "panelsettings.h"

LXQtPlugin::LXQtPlugin(const ILXQtPanelPluginStartupInfo &startupInfo) :
    QObject(),
    ILXQtPanelPlugin(startupInfo),
    proc(&dbus)
{
    wrapper = new QWidget();
    auto layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setContentsMargins(0, 0, 0, 0);
    wrapper->setLayout(layout);

    fakePopup = new QWidget();
    fakePopup->setMaximumSize(0, 0);
    fakePopup->move(-200, -200);
    fakePopup->hide();

    connect(&dbus, &DBusProxy::ready, this, &LXQtPlugin::onReady);
    connect(&dbus, &DBusProxy::sizeChanged, this, &LXQtPlugin::onSizeChanged);
    connect(&dbus, &DBusProxy::popup, this, &LXQtPlugin::onPopup);
}

LXQtPlugin::~LXQtPlugin() {
    delete settings;
    delete fakePopup;
    delete wrapper;
}

QWidget *LXQtPlugin::widget() {
    return wrapper;
}

QDialog *LXQtPlugin::configureDialog() {
    return new ConfigDialog();
}

void LXQtPlugin::realign() {
    QString orient = getOrient();
    int size = getPanelSize();
    if (settings == nullptr) {
        settings = new PanelSettings(wrapper->window()->objectName());
        connect(settings, &PanelSettings::backgroundChanged, this, &LXQtPlugin::onBackgroundChanged);
        remoteOrient = orient;
        remoteSize = size;
        proc.setStartupArguments(remoteOrient, remoteSize);
        proc.start();
        return;
    }
    if (remoteOrient != orient) {
        if (dbus.callSetOrient(orient)) {
            remoteOrient = orient;
        }
    }
    if (remoteSize != size) {
        if (dbus.callSetSize(size)) {
            remoteSize = size;
        }
    }
    proc.setStartupArguments(remoteOrient, remoteSize);
    QPoint pos = wrapper->mapToGlobal(QPoint(0, 0));
    if (this->pos != pos) {
        this->pos = pos;
        if (settings->getBackgroundImage().isEmpty() == false) {
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
    QString image = settings->getBackgroundImage();
    QColor color = settings->getBackgroundColor();
    int opacity = settings->getOpacity();
    onBackgroundChanged(image, color, opacity);
}

void LXQtPlugin::onReady(uint winId) {
    QWindow *win = QWindow::fromWinId(winId);
    QLayout *layout = wrapper->layout();
    if (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        delete item->widget();
        delete item;
    }
    layout->addWidget(QWidget::createWindowContainer(win, nullptr, Qt::ForeignWindow));
    setBackground();
}

void LXQtPlugin::onSizeChanged(int width, int height) {
    wrapper->setMinimumSize(width, height);
}

void LXQtPlugin::onPopup(bool shown) {
    if (shown) {
        panel()->willShowWindow(fakePopup);
        fakePopup->show();
    } else {
        fakePopup->hide();
    }
}

void LXQtPlugin::onBackgroundChanged(const QString &image, const QColor &color, int opacity) {
    int offsetX, offsetY;
    if (image.isEmpty() == false) {
        QPoint pluginPos = wrapper->mapToGlobal(QPoint(0, 0));
        QRect panelPos = panel()->globalGeometry();
        offsetX = pluginPos.x() - panelPos.x();
        offsetY = pluginPos.y() - panelPos.y();
    } else {
        offsetX = offsetY = 0;
    }
    QColor c;
    if (color.isValid()) {
        c = color;
        c.setAlpha(opacity);
    } else {
#if QT_VERSION_MAJOR >= 6
        QWidget widget;
        c = widget.palette().color(widget.backgroundRole());
#else
        c.setAlpha(0);
#endif
    }
    QString rgba = QStringLiteral(u"rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha() / 100.0);
    dbus.callSetBackground(rgba, image, offsetX, offsetY);
}
