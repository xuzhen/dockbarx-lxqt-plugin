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

    void setMaxSize(int value);
    int getMaxSize();

    void setMaxSizeEnabled(bool enabled);
    bool isMaxSizeEnabled();

    int getEnabledMaxSize();

signals:
    void offsetChanged(int offset);
    void maxSizeChanged(int maxSize);

private:
    PluginSettings *settings;

    int offset;
    int maxSize;
    bool maxSizeEnabled;
};

#endif // LXQTPLUGINSETTINGS_H
