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
#include "configdialog.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QProcess>
#include "lxqtpluginsettings.h"


const QString ConfigDialog::dbx_pref = QStringLiteral(u"dbx_preference");

ConfigDialog::ConfigDialog(LXQtPluginSettings *settings, QWidget *parent) : QDialog(parent), settings(settings) {
    setWindowTitle(tr("DockbarX Plugin Preferences"));

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    QGridLayout *optionsLayout = new QGridLayout();

    offsetBox = new QSpinBox();
    offsetBox->setMinimum(-100);
    offsetBox->setMaximum(100);
    optionsLayout->addWidget(new QLabel(tr("Icon Offset")), 0, 1);
    optionsLayout->addWidget(offsetBox, 0, 2);

#ifdef ENABLE_SET_MAX_SIZE
    maxSizeCheck = new QCheckBox();
    maxSizeBox = new QSpinBox();
    maxSizeBox->setMinimum(100);
    maxSizeBox->setMaximum(QWIDGETSIZE_MAX);
    optionsLayout->addWidget(maxSizeCheck, 1, 0);
    optionsLayout->addWidget(new QLabel(tr("Max Size")), 1, 1);
    optionsLayout->addWidget(maxSizeBox, 1, 2);
#endif

    optionsLayout->setColumnStretch(0, 0);
    optionsLayout->setColumnStretch(1, 0);
    optionsLayout->setColumnStretch(2, 1);

    layout->addLayout(optionsLayout);

    buttons = new QDialogButtonBox();
    buttons->addButton(QDialogButtonBox::Close);
    buttons->addButton(tr("Open DockbarX Preferences"), QDialogButtonBox::ActionRole);
    layout->addWidget(buttons);

    connect(offsetBox, &QSpinBox::valueChanged, this, &ConfigDialog::updateOffset);
#ifdef ENABLE_SET_MAX_SIZE
    connect(maxSizeBox, &QSpinBox::valueChanged, this, &ConfigDialog::updateMaxSize);
    connect(maxSizeCheck, &QCheckBox::stateChanged, this, &ConfigDialog::onCheck);
#endif
    connect(buttons, &QDialogButtonBox::clicked, this, &ConfigDialog::onButton);

    initSettings();
}

void ConfigDialog::initSettings() {
    offsetBox->setValue(settings->getOffset());
#ifdef ENABLE_SET_MAX_SIZE
    maxSizeBox->setValue(settings->getMaxSize());
    bool enable = settings->isMaxSizeEnabled();
    maxSizeCheck->setChecked(enable);
    maxSizeBox->setEnabled(enable);
#endif
}

void ConfigDialog::updateOffset(int value) {
    settings->setOffset(value);
}

#ifdef ENABLE_SET_MAX_SIZE
void ConfigDialog::updateMaxSize(int value) {
    settings->setMaxSize(value);
}

void ConfigDialog::onCheck() {
    bool enabled = (maxSizeCheck->checkState() == Qt::Checked);
    settings->setMaxSizeEnabled(enabled);
    maxSizeBox->setEnabled(enabled);
}
#endif

void ConfigDialog::onButton(QAbstractButton *button) {
    auto role = buttons->buttonRole(button);
    if (role == QDialogButtonBox::RejectRole) {
        this->close();
    } else if (role == QDialogButtonBox::ActionRole) {
        QProcess::startDetached(dbx_pref);
    }
}
