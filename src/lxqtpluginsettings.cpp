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
#include "lxqtpluginsettings.h"
#include <lxqt/pluginsettings.h>

LXQtPluginSettings::LXQtPluginSettings(PluginSettings *settings, QObject *parent) : QObject(parent), settings(settings) {
    iconSize = settings->value(QStringLiteral(u"IconSize"), QVariant(32)).toInt();
    iconSizeEnabled = settings->value(QStringLiteral(u"SetIconSize"), QVariant(false)).toBool();

    offset = settings->value(QStringLiteral(u"Offset"), QVariant(0)).toInt();

#ifdef ENABLE_SET_MAX_SIZE
    maxSize = settings->value(QStringLiteral(u"MaxSize"), QVariant(500)).toInt();
    maxSizeEnabled = settings->value(QStringLiteral(u"SetMaxSize"), QVariant(false)).toBool();
#endif
}

void LXQtPluginSettings::setIconSize(int value) {
    if (value == iconSize) {
        return;
    }
    iconSize = value;
    settings->setValue(QStringLiteral(u"IconSize"), QVariant(value));
    if (iconSizeEnabled) {
        emit iconSizeChanged(value);
    }
}

int LXQtPluginSettings::getIconSize() {
    return iconSize;
}

void LXQtPluginSettings::setIconSizeEnabled(bool enabled) {
    if (enabled == iconSizeEnabled) {
        return;
    }
    iconSizeEnabled = enabled;
    settings->setValue(QStringLiteral(u"SetIconSize"), QVariant(enabled));
    if (enabled) {
        emit iconSizeChanged(iconSize);
    } else {
        emit iconSizeChanged(-1);
    }
}

bool LXQtPluginSettings::isIconSizeEnabled() {
    return iconSizeEnabled;
}

int LXQtPluginSettings::getEnabledIconSize() {
    if (iconSizeEnabled) {
        return iconSize;
    } else {
        return -1;
    }
}

void LXQtPluginSettings::setOffset(int value) {
    if (value == offset) {
        return;
    }
    offset = value;
    settings->setValue(QStringLiteral(u"Offset"), QVariant(value));
    emit offsetChanged(value);
}

int LXQtPluginSettings::getOffset() {
    return offset;
}

#ifdef ENABLE_SET_MAX_SIZE
void LXQtPluginSettings::setMaxSize(int value) {
    if (value == maxSize) {
        return;
    }
    maxSize = value;
    settings->setValue(QStringLiteral(u"MaxSize"), QVariant(value));
    if (maxSizeEnabled) {
        emit maxSizeChanged(value);
    }
}

int LXQtPluginSettings::getMaxSize() {
    return maxSize;
}

void LXQtPluginSettings::setMaxSizeEnabled(bool enabled) {
    if (enabled == maxSizeEnabled) {
        return;
    }
    maxSizeEnabled = enabled;
    settings->setValue(QStringLiteral(u"SetMaxSize"), QVariant(enabled));
    if (enabled) {
        emit maxSizeChanged(maxSize);
    } else {
        emit maxSizeChanged(-1);
    }
}

bool LXQtPluginSettings::isMaxSizeEnabled() {
    return maxSizeEnabled;
}

int LXQtPluginSettings::getEnabledMaxSize() {
    if (maxSizeEnabled) {
        return maxSize;
    } else {
        return -1;
    }
}
#endif
