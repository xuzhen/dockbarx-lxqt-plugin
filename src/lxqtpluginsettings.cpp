#include "lxqtpluginsettings.h"
#include <lxqt/pluginsettings.h>

LXQtPluginSettings::LXQtPluginSettings(PluginSettings *settings, QObject *parent) : QObject(parent), settings(settings) {
    offset = settings->value(QStringLiteral(u"Offset"), QVariant(0)).toInt();
#ifdef ENABLE_SET_MAX_SIZE
    maxSize = settings->value(QStringLiteral(u"MaxSize"), QVariant(500)).toInt();
    maxSizeEnabled = settings->value(QStringLiteral(u"SetMaxSize"), QVariant(false)).toBool();
#endif
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
    if (maxSizeEnabled) {
        return maxSize;
    } else {
        return -1;
    }
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