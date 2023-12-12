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
    explicit PanelSettings(QObject *parent = nullptr);
    ~PanelSettings();

    int getPanelSize();
    int getOpacity();
    QColor getBackgroundColor();
    QString getBackgroundImage();

signals:
    void backgroundChanged(QString image, QColor color, int opacity);

private slots:
    void modified();

private:
    void initValues();
    int readPanelSize();
    int readOpacity();
    QColor readBackgroundColor();
    QString readBackgroundImage();

    QSettings *settings;

    int size;
    int opacity;
    QColor color;
    QString image;

    PanelSettingsWatcher *watcher;
};

#endif // PANELSETTINGS_H
