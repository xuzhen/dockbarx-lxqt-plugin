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
class LXQtPanel;
class LXQtPluginSettings;
class QBoxLayout;
class QScreen;

class DockbarContainer : public QWidget
{
    Q_OBJECT
public:
    explicit DockbarContainer(ILXQtPanel *panel, LXQtPluginSettings *settings, QWidget *parent = nullptr);
    virtual ~DockbarContainer();

    void capture(QWindow *window);
    void updateDirection();
    void updateSize();
    bool updateMargins();

    int getMargin();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QScreen *screen() const;
#endif

public slots:
    void setIconOffset(int offset);
#ifdef ENABLE_SET_MAX_SIZE
    void setMaxSize(int size);
#endif

private:
    LXQtPanel *panel;
    QBoxLayout *layout;
    int margin = 0;
    int iconOffset = 0;
    int maxSize = QWIDGETSIZE_MAX;
};

#endif // DOCKBARCONTAINER_H
