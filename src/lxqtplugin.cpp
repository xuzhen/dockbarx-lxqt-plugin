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
#include <QApplication>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QScreen>
#include "configdialog.h"
#include "panelsettings.h"
#include "dockbarcontainer.h"

LXQtPlugin::LXQtPlugin(const ILXQtPanelPluginStartupInfo &startupInfo) :
    QObject(),
    ILXQtPanelPlugin(startupInfo),
    wrapper(panel()),
    proc(wrapper.screen())
{
    connect(&proc, &PyAppletKeeper::dockReady, this, &LXQtPlugin::onReady);
    connect(&proc, &PyAppletKeeper::dockSizeChanged, this, &LXQtPlugin::onSizeChanged);
    connect(&proc, &PyAppletKeeper::dockPopup, this, &LXQtPlugin::onPopup);
}

LXQtPlugin::~LXQtPlugin() {
    delete settings;
    delete fakePopup;
}

QWidget *LXQtPlugin::widget() {
    return &wrapper;
}

QDialog *LXQtPlugin::configureDialog() {
    return new ConfigDialog();
}

void LXQtPlugin::realign() {
    QString orient = getOrient();
    int size = getPanelSize();
    if (settings == nullptr) {
        settings = new PanelSettings(wrapper.window()->objectName());
        connect(settings, &PanelSettings::backgroundChanged, this, &LXQtPlugin::onBackgroundChanged);
        connect(settings, &PanelSettings::iconThemeChanged, this, &LXQtPlugin::onIconThemeChanged);
        // As of version 2.0.1, lxqt-panel still won't call realign() when panel position changed
        connect(settings, &PanelSettings::positionChanged, this, &LXQtPlugin::realign);
        proc.setDockOrient(orient);
        proc.setDockSize(size);
        proc.setDockIconTheme(settings->getIconTheme());
        proc.start();
        return;
    }
    bool updateBackground = false;
    bool updateSize = false;
    if (proc.setDockOrient(orient)) {
        wrapper.updateDirection();
        updateSize = true;
    }
    if (proc.setDockSize(size)) {
        updateSize = true;
        updateBackground = true;
    }
    if (updateSize) {
        wrapper.updateSize();
    }
    QPoint pos = wrapper.mapToGlobal(QPoint(0, 0));
    if (this->pos != pos) {
        this->pos = pos;
        updateBackground = true;
    }
    if (updateBackground && !settings->isFixedBackground()) {
        setBackground();
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
    QString color = settings->getBackgroundColor();
    onBackgroundChanged(image, color);
}


void LXQtPlugin::onReady(uint winId) {
    QWindow *win = QWindow::fromWinId(winId);
    wrapper.capture(win);
    setBackground();
}

void LXQtPlugin::onSizeChanged(int width, int height) {
    if (panel()->isHorizontal()) {
        wrapper.setMinimumWidth(width);
    } else {
        wrapper.setMinimumHeight(height);
    }
}

void LXQtPlugin::onPopup(bool shown) {
    if (fakePopup == nullptr) {
        fakePopup = new QWidget();
        fakePopup->setMaximumSize(0, 0);
        fakePopup->move(-200, -200);
    }
    if (shown) {
        panel()->willShowWindow(fakePopup);
        fakePopup->show();
    } else {
        fakePopup->hide();
    }
}

void LXQtPlugin::onBackgroundChanged(const QString &image, const QString &color) {
    int offsetX, offsetY, panelWidth, panelHeight;
    if (settings->isFixedBackground() == false) {
        QRect panelGeo = panel()->globalGeometry();
        QPoint pluginPos = wrapper.mapToGlobal(QPoint(0, 0));
        offsetX = pluginPos.x() - panelGeo.x();
        offsetY = pluginPos.y() - panelGeo.y();
        panelWidth = panelGeo.width();
        panelHeight = panelGeo.height();
    } else {
        offsetX = offsetY = panelWidth = panelHeight = 0;
    }
    proc.setDockBackground(color, image, offsetX, offsetY, panelWidth, panelHeight);
}

void LXQtPlugin::onIconThemeChanged(const QString &themeName) {
    proc.setDockIconTheme(themeName);
}
