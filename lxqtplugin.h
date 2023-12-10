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
#ifndef LXQTPLUGIN_H
#define LXQTPLUGIN_H

#include <QObject>
#include <lxqt/ilxqtpanelplugin.h>

class QWidget;
class QWindow;
class QDBusInterface;

class LXQT_PANEL_API LXQtPlugin : public QObject, public ILXQtPanelPlugin
{
    Q_OBJECT
public:
    LXQtPlugin(const ILXQtPanelPluginStartupInfo &startupInfo);
    virtual ~LXQtPlugin();

    QWidget *widget() override;
    QString themeId() const override { return QStringLiteral("DockbarX"); }
    Flags flags() const override { return SingleInstance | HaveConfigDialog; }

    QDialog *configureDialog() override;

    void realign() override;

private slots:
    void onReady(uint winId);
    void onSizeChanged(QList<int> size);

private:
    QString getOrient();
    int getPanelSize();

    bool runPythonApplet();
    bool prepareDBus();

    bool dbusSetSize(int size);
    bool dbusSetOrient(const QString &orient);

    QString remoteOrient;
    int remoteSize;
    QWidget *wrapper;
    QDBusInterface *iface;
};


#endif // LXQTPLUGIN_H
