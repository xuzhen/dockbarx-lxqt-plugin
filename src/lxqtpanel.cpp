/*
 Copyright (C) 2024 Xu Zhen

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
#include "lxqtpanel.h"
#include <lxqt/ilxqtpanel.h>
#include "lxqtpluginsettings.h"

LXQtPanel::LXQtPanel(ILXQtPanel *panel, LXQtPluginSettings *settings) : panel(panel), settings(settings) {
}

int LXQtPanel::size() const {
    QRect geo = panel->globalGeometry();
    if (panel->isHorizontal()) {
        return geo.height();
    } else {
        return geo.width();
    }
}

QString LXQtPanel::orient() const {
    ILXQtPanel::Position pos = panel->position();
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

int LXQtPanel::iconSize() const {
    if (settings->isIconSizeEnabled()) {
        int iconSize = settings->getIconSize();
        if (iconSize > 0) {
            return std::min(iconSize, size());
        }
    }
    return std::min(panel->iconSize(), size());
}

bool LXQtPanel::isHorizontal() const {
    return panel->isHorizontal();
}
