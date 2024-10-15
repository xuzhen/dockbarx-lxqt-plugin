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
#include "lxqtpanel.h"
#include <QBoxLayout>
#include <QWindow>

DockbarContainer::DockbarContainer(ILXQtPanel *panel, QWidget *parent) : QWidget(parent) {
    this->panel = new LXQtPanel(panel);
    layout = new QBoxLayout(QBoxLayout::RightToLeft);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
}

DockbarContainer::~DockbarContainer() {
    delete panel;
}

void DockbarContainer::capture(QWindow *window) {
    if (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        delete item->widget();
        delete item;
    }
    layout->addWidget(QWidget::createWindowContainer(window, nullptr, Qt::ForeignWindow));
    updateDirection();
    updateSize();
}

void DockbarContainer::updateDirection() {
    auto newDirection = panel->isHorizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
    if (layout->direction() != newDirection) {
        layout->setDirection(newDirection);
    }
}

void DockbarContainer::updateSize() {
    int size = panel->size();
    if (layout->direction() == QBoxLayout::LeftToRight) {
        setMinimumHeight(size);
        setMaximumSize(QWIDGETSIZE_MAX, size);
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    } else {
        setMinimumWidth(size);
        setMaximumSize(size, QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    }
}

bool DockbarContainer::updateMargins() {
    int iconSize = panel->iconSize();
    int panelSize = panel->size();
    int marginStart = (panelSize - iconSize) / 2;
    int marginEnd = panelSize - iconSize - marginStart;
    if (panel->isHorizontal()) {
        layout->setContentsMargins(0, marginStart, 0, marginEnd);
    } else {
        layout->setContentsMargins(marginStart, 0, marginEnd, 0);
    }
    if (margin != marginStart) {
        margin = marginStart;
        return true;
    }
    return false;
}

int DockbarContainer::getMargin() {
    return margin;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
QScreen *DockbarContainer::screen() const {
    winId();   // ensure wrapper->windowHandle() returns a valid QWindow
    return windowHandle()->screen();
}
#endif