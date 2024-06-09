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
#include "dockbarcontainer.h"
#include <lxqt/ilxqtpanel.h>
#include <QBoxLayout>

DockbarContainer::DockbarContainer(ILXQtPanel *panel, QWidget *parent) : QWidget(parent), panel(panel) {
    layout = new QBoxLayout(QBoxLayout::RightToLeft);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
}

void DockbarContainer::capture(QWindow *window) {
    if (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        delete item->widget();
        delete item;
    }
    layout->addWidget(QWidget::createWindowContainer(window, nullptr, Qt::ForeignWindow));
    updateDirection();
}

void DockbarContainer::updateDirection() {
    auto newDirection = panel->isHorizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
    if (layout->direction() != newDirection) {
        layout->setDirection(newDirection);
        if (newDirection == QBoxLayout::LeftToRight) {
            int size = panel->globalGeometry().height();
            setMinimumSize(size, size);
            setMaximumSize(QWIDGETSIZE_MAX, size);
            setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        } else {
            int size = panel->globalGeometry().width();
            setMinimumSize(size, size);
            setMaximumSize(size, QWIDGETSIZE_MAX);
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
        }
    }
}
