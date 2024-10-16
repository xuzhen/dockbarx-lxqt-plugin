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

#ifdef ENABLE_SET_MAX_SIZE
    void setMaxSize(int value);
    int getMaxSize();

    void setMaxSizeEnabled(bool enabled);
    bool isMaxSizeEnabled();

    int getEnabledMaxSize();
#endif

signals:
    void offsetChanged(int offset);
#ifdef ENABLE_SET_MAX_SIZE
    void maxSizeChanged(int maxSize);
#endif

private:
    PluginSettings *settings;

    int offset;
#ifdef ENABLE_SET_MAX_SIZE
    int maxSize;
    bool maxSizeEnabled;
#endif
};

#endif // LXQTPLUGINSETTINGS_H
