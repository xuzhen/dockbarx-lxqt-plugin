#ifndef PANELSETTINGS_H
#define PANELSETTINGS_H

#include <QObject>
#include <QColor>

class QSettings;
class PanelSettingsWatcher;

class PanelSettings : public QObject
{
    Q_OBJECT
public:
    explicit PanelSettings(const QString &panelName, QObject *parent = nullptr);
    ~PanelSettings();

    int getOpacity();
    QColor getBackgroundColor();
    QString getBackgroundImage();

signals:
    void backgroundChanged(QString image, QColor color, int opacity);

private slots:
    void modified();

private:
    QString findSettingFile();
    void setSettingGroup(const QString &panelName);
    void initValues();
    int readOpacity();
    QColor readBackgroundColor();
    QString readBackgroundImage();

    QSettings *settings;

    int opacity;
    QColor color;
    QString image;

    PanelSettingsWatcher *watcher;
};

#endif // PANELSETTINGS_H
