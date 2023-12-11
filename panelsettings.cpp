#include "panelsettings.h"
#include <QStandardPaths>
#include <QSettings>
#include <QColor>
#include <QThread>
#include "panelsettingswatcher.h"

PanelSettings::PanelSettings(QObject *parent) : QObject(parent) {
    QString file = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral(u"/lxqt/panel.conf");
    settings = new QSettings(file, QSettings::IniFormat);
    settings->beginGroup(QStringLiteral(u"panel1"));
    initValues();

    watcher = new PanelSettingsWatcher(file);
    QThread *thread = new QThread();
    watcher->moveToThread(thread);
    connect(thread, &QThread::started, watcher, &PanelSettingsWatcher::start);
    connect(watcher, &PanelSettingsWatcher::destroyed, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(watcher, &PanelSettingsWatcher::modified, this, &PanelSettings::modified);
    thread->start();
}

PanelSettings::~PanelSettings() {
    watcher->stop();
    watcher->deleteLater();
    delete settings;
}

int PanelSettings::getPanelSize() {
    return size;
}

int PanelSettings::getOpacity() {
    return opacity;
}

QColor PanelSettings::getBackgroundColor() {
    return color;
}

QString PanelSettings::getBackgroundImage() {
    return image;
}

void PanelSettings::modified() {
    QString image = readBackgroundImage();
    QColor color = readBackgroundColor();
    int opacity = readOpacity();
    bool changed = false;
    if (this->image != image || this->color != color || this->opacity != opacity) {
        this->image = image;
        this->color = color;
        this->opacity = opacity;
        emit backgroundChanged(image, color, opacity);
    }
}

void PanelSettings::initValues() {
    size = readPanelSize();
    opacity = readOpacity();
    color = readBackgroundColor();
    image = readBackgroundImage();
}

int PanelSettings::readPanelSize() {
    return settings->value(QStringLiteral(u"panelSize"), QVariant(32)).toInt();
}

int PanelSettings::readOpacity() {
    return settings->value(QStringLiteral(u"opacity"), QVariant(100)).toInt();
}

QColor PanelSettings::readBackgroundColor() {
    return settings->value(QStringLiteral(u"background-color"), QVariant(QColor())).value<QColor>();
}

QString PanelSettings::readBackgroundImage() {
    return settings->value(QStringLiteral(u"background-image")).toString();
}
