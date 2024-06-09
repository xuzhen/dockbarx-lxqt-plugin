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

#ifndef DOCKBARCONTAINER_H
#define DOCKBARCONTAINER_H

#include <QWidget>

class ILXQtPanel;
class QBoxLayout;

class DockbarContainer : public QWidget
{
    Q_OBJECT
public:
    explicit DockbarContainer(ILXQtPanel *panel, QWidget *parent = nullptr);
    void capture(QWindow *window);
    void updateDirection();

private:
    ILXQtPanel *panel;
    QBoxLayout *layout;
};

#endif // DOCKBARCONTAINER_H
