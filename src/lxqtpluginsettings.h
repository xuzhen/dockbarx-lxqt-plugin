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
#ifndef LXQTPLUGINSETTINGS_H
#define LXQTPLUGINSETTINGS_H

#include <QObject>

class PluginSettings;

class LXQtPluginSettings : public QObject
{
    Q_OBJECT
public:
    explicit LXQtPluginSettings(PluginSettings *settings, QObject *parent = nullptr);

    void setOffset(int value);
    int getOffset();

#ifdef ENABLE_SET_MAX_SIZE
    void setMaxSize(int value);
    int getMaxSize();

    void setMaxSizeEnabled(bool enabled);
    bool isMaxSizeEnabled();

    int getEnabledMaxSize();
#endif

signals:
    void offsetChanged(int offset);
#ifdef ENABLE_SET_MAX_SIZE
    void maxSizeChanged(int maxSize);
#endif

private:
    PluginSettings *settings;

    int offset;
#ifdef ENABLE_SET_MAX_SIZE
    int maxSize;
    bool maxSizeEnabled;
#endif
};

#endif // LXQTPLUGINSETTINGS_H
