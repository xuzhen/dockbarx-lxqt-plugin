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
#ifndef LXQTPANEL_H
#define LXQTPANEL_H

#include <QString>

class ILXQtPanel;
class LXQtPluginSettings;

class LXQtPanel
{
public:
    LXQtPanel(ILXQtPanel *panel, LXQtPluginSettings *settings);

    int size() const;
    QString orient() const;

    int iconSize() const;
    bool isHorizontal() const;

private:
    ILXQtPanel *panel;
    LXQtPluginSettings *settings;
};

#endif // LXQTPANEL_H
